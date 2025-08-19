// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Implementation details and notes:
// - Overlap handlers call into ApplyAllGameplayEffects/RemoveAllGameplayEffects with the appropriate policy.
// - For application, we build an effect context with this actor as source/causer and apply to the target's ASC.
// - For Infinite effects (with a non-DoNotRemove removal policy), we record the returned handle per target ASC,
//   enabling precise removal later. Instant effects return invalid handles by design.
// - Removal traverses tracked handles for the target ASC and removes stacks based on StacksToRemove.
// - Verbose logging is included to aid debugging and tuning.
//
// Authority:
// - Consider gating overlap-driven application/removal behind HasAuthority() checks depending on your game.
//   Applying/removing GEs is usually server-authoritative in multiplayer.

#include "Actors/CoreGameplayEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

ACoreGameplayEffectActor::ACoreGameplayEffectActor()
{
	// Provide a neutral root so designers can add collision/visuals in BP as needed.
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	SetRootComponent(DefaultSceneRoot);
}

void ACoreGameplayEffectActor::BeginPlay()
{
	Super::BeginPlay();

	// Validate configuration: warn if the same Infinite GE class appears multiple times.
	// Duplicate Infinite classes can make handle-based tracking/removal ambiguous.
	TSet<TSubclassOf<UGameplayEffect>> SeenInfiniteClasses;
	for (const FCoreEffectConfig& Config : GameplayEffects)
	{
		if (IsInfinite(Config.EffectClass))
		{
			SeenInfiniteClasses.Add(Config.EffectClass);
		}
	}

	// Log a summary of configured effects for quick validation/tuning.
	for (const FCoreEffectConfig& Config : GameplayEffects)
	{
		if (!Config.EffectClass) continue;

		const EGameplayEffectDurationType DurationPolicy = GetDurationPolicyOf(Config.EffectClass);
		const bool bIsPeriodic = IsPeriodic(Config.EffectClass);
		const TCHAR* TypeStr =
			(DurationPolicy == EGameplayEffectDurationType::Instant)  ? TEXT("Instant")  :
			(DurationPolicy == EGameplayEffectDurationType::Infinite) ? TEXT("Infinite") :
			(bIsPeriodic ? TEXT("Periodic") : TEXT("Duration"));
	}
}

void ACoreGameplayEffectActor::OnOverlap(AActor* TargetActor)
{
	if (!TargetActor) return;

	// On overlap:
	// - Apply any effects configured for ApplyOnOverlap.
	// - Optionally remove any effects configured for RemoveOnOverlap (less common, but supported).
	ApplyAllGameplayEffects(TargetActor, ECoreEffectApplicationPolicy::ApplyOnOverlap);
	RemoveAllGameplayEffects(TargetActor, ECoreEffectRemovalPolicy::RemoveOnOverlap);
}

void ACoreGameplayEffectActor::EndOverlap(AActor* TargetActor)
{
	if (!TargetActor) return;

	// On end overlap:
	// - Apply any effects configured for ApplyEndOverlap (grant-on-exit designs).
	// - Remove any effects configured for RemoveOnEndOverlap (typical AoE cleanup).
	ApplyAllGameplayEffects(TargetActor, ECoreEffectApplicationPolicy::ApplyEndOverlap);
	RemoveAllGameplayEffects(TargetActor, ECoreEffectRemovalPolicy::RemoveOnEndOverlap);
}

void ACoreGameplayEffectActor::ApplyAllGameplayEffects(AActor* TargetActor, ECoreEffectApplicationPolicy ApplicationPolicy)
{
	// Iterate through configured effects and apply those whose ApplicationPolicy matches.
	for (const FCoreEffectConfig& Config : GameplayEffects)
	{
		if (Config.EffectClass && Config.ApplicationPolicy == ApplicationPolicy)
		{
			ApplyGameplayEffectToTarget(TargetActor, Config);
		}
	}
}

void ACoreGameplayEffectActor::RemoveAllGameplayEffects(AActor* TargetActor, ECoreEffectRemovalPolicy RemovalPolicy)
{
	// Iterate through configured effects and remove those whose RemovalPolicy matches.
	for (const FCoreEffectConfig& Config : GameplayEffects)
	{
		if (Config.EffectClass && Config.RemovalPolicy == RemovalPolicy)
		{
			RemoveGameplayEffectFromTarget(TargetActor, Config);
		}
	}
}

void ACoreGameplayEffectActor::ApplyGameplayEffectToTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig)
{
	// 1) Resolve the target ASC (supports IAbilitySystemInterface or direct component search).
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	// 2) Sanity-check the effect class (should be set in config).
	checkf(EffectConfig.EffectClass, TEXT("EffectClass is unset on %s"), *GetName());

	// 3) Build an effect context originating from this actor, using the target ASC as the creator.
	//    - AddSourceObject(this) allows consumers to trace the source effect actor.
	//    - SetEffectCauser(this) marks this actor as the causer (useful for damage attribution, etc.).
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	if (EffectContextHandle.Get())
	{
		EffectContextHandle.Get()->SetEffectCauser(this);
	}

	// 4) Build a spec for this effect at the configured level, with the context provided.
	const FGameplayEffectSpecHandle GameplayEffectSpecHandle =
		TargetASC->MakeOutgoingSpec(EffectConfig.EffectClass, EffectConfig.ActorLevel, EffectContextHandle);

	// 5) Apply the spec to the target ASC (apply-to-self on that ASC).
	//    Note: For Instant effects, the returned handle is typically invalid (INDEX_NONE), which is expected.
	const FActiveGameplayEffectHandle ActiveEffectHandle =
		TargetASC->ApplyGameplayEffectSpecToSelf(*GameplayEffectSpecHandle.Data.Get());

	// 6) If this is an Infinite effect and we plan to remove it later, track the handle per target ASC.
	if (IsInfinite(EffectConfig.EffectClass)
		&& EffectConfig.RemovalPolicy != ECoreEffectRemovalPolicy::DoNotRemove
		&& ActiveEffectHandle.IsValid())
	{
		ActiveInfiniteEffects.Add(ActiveEffectHandle, TargetASC);
	}

	// 7) Optional: destroy the effect actor right after a successful application.
	if (EffectConfig.bDestroyOnEffectApplication)
	{
		Destroy();
	}
}

void ACoreGameplayEffectActor::RemoveGameplayEffectFromTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig)
{
	// 1) Resolve the target ASC.
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	// 2) Collect handles:
	//    - Only those we previously recorded in ActiveInfiniteEffects
	//    - Which belong to THIS TargetASC
	//    - And whose active spec.Def class matches EffectConfig.EffectClass
	//    Stale handles (no longer active) are also collected for cleanup.
	TArray<FActiveGameplayEffectHandle> HandlesForThisASC;
	for (const TPair<FActiveGameplayEffectHandle, UAbilitySystemComponent*>& TrackedPair : ActiveInfiniteEffects)
	{
		const FActiveGameplayEffectHandle TrackedHandle = TrackedPair.Key;
		UAbilitySystemComponent* ASCForHandle = TrackedPair.Value;

		if (ASCForHandle != TargetASC)
		{
			continue; // Different target ASC.
		}

		const FActiveGameplayEffect* ActiveEffect = TargetASC->GetActiveGameplayEffect(TrackedHandle);
		if (!ActiveEffect)
		{
			// Stale entry; add for cleanup.
			HandlesForThisASC.Add(TrackedHandle);
			continue;
		}

		if (ActiveEffect->Spec.Def && ActiveEffect->Spec.Def->GetClass() == EffectConfig.EffectClass)
		{
			HandlesForThisASC.Add(TrackedHandle);
		}
	}

	// 3) Remove stacks/effects for all matching handles and note if anything was removed.
	bool bRemovedAnyStacks = false;

	for (const FActiveGameplayEffectHandle& Handle : HandlesForThisASC)
	{
		const FActiveGameplayEffect* ExistingActiveEffect = TargetASC->GetActiveGameplayEffect(Handle);
		if (!ExistingActiveEffect)
		{
			// Already stale, skip removal; cleanup will erase it below.
			continue;
		}

		// RemoveActiveGameplayEffect returns number of stacks removed (0 if nothing removed).
		// StacksToRemove: -1 = remove all stacks; 1 = remove a single stack.
		const int32 RemovedCount = TargetASC->RemoveActiveGameplayEffect(Handle, EffectConfig.StacksToRemove);
		bRemovedAnyStacks |= (RemovedCount != 0);
	}

	// 4) Cleanup: erase handles that no longer exist on the ASC.
	for (const FActiveGameplayEffectHandle& Handle : HandlesForThisASC)
	{
		if (!TargetASC->GetActiveGameplayEffect(Handle))
		{
			ActiveInfiniteEffects.Remove(Handle);
		}
	}

	// 5) Optional: destroy the effect actor after a successful removal.
	if (bRemovedAnyStacks && EffectConfig.bDestroyOnEffectRemoval)
	{
		Destroy();
	}
}

// ------------ Helper queries on GE classes ------------

EGameplayEffectDurationType ACoreGameplayEffectActor::GetDurationPolicyOf(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	if (!EffectClass) return EGameplayEffectDurationType::Instant;

	const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
	return EffectCDO ? EffectCDO->DurationPolicy : EGameplayEffectDurationType::Instant;
}

bool ACoreGameplayEffectActor::IsPeriodic(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	if (!EffectClass) return false;

	const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
	return EffectCDO && EffectCDO->Period.GetValue() > 0.f;
}

bool ACoreGameplayEffectActor::IsInfinite(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	return GetDurationPolicyOf(EffectClass) == EGameplayEffectDurationType::Infinite;
}