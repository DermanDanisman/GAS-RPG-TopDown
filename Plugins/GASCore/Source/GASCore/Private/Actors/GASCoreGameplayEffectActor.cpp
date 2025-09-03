// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Implementation details and notes:
// - Overlap handlers call ApplyAllGameplayEffects/RemoveAllGameplayEffects with the appropriate policy.
// - For application, we build an effect context with this actor as source/causer and apply to the target's ASC.
// - For non-instant effects (HasDuration or Infinite; Periodic counts as non-instant) with a removal policy,
//   we record the returned handle per target ASC (tracked via TWeakObjectPtr), enabling precise, GC-safe removal.
// - Removal traverses tracked handles for the target ASC, matches by effect class, and removes stacks
//   based on the stacks value recorded at application time (per-handle, not per-config at removal).
//
// Multiplayer authority:
// - In networked games, prefer guarding OnOverlap/EndOverlap with HasAuthority() or perform server RPCs.

#include "GASCore/Public/Actors/GASCoreGameplayEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

AGASCoreGameplayEffectActor::AGASCoreGameplayEffectActor()
{
	SetReplicates(true);
	// Provide a neutral root so designers can add collision/visuals in BP as needed.
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	SetRootComponent(DefaultSceneRoot);
}

void AGASCoreGameplayEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGASCoreGameplayEffectActor::OnOverlap(AActor* TargetActor)
{
	// Early-out for safety: we need a target actor to proceed.
	if (!TargetActor) return;

	// OVERLAP LIFECYCLE MAPPING:
	// On overlap entry, we apply effects configured for ApplyOnOverlap and remove effects configured for RemoveOnOverlap.
	// 
	// TYPICAL APPLICATION POLICIES:
	// - ApplyOnOverlap: Most common (health potions, buffs, fire area damage)
	// - ApplyEndOverlap: Less common (grant-on-exit mechanics, reverse triggers)
	// - DoNotApply: Manual control only
	//
	// TYPICAL REMOVAL POLICIES (for Infinite effects):
	// - RemoveOnEndOverlap: Most common (fire areas, aura buffs that end when leaving)
	// - RemoveOnOverlap: Rare (toggle behaviors, entry-triggered removal)
	// - DoNotRemove: Permanent effects or manual control
	
	ApplyAllGameplayEffects(TargetActor, EGASCoreEffectApplicationPolicy::ApplyOnOverlap);
	RemoveAllGameplayEffects(TargetActor, EGASCoreEffectRemovalPolicy::RemoveOnOverlap);
}

void AGASCoreGameplayEffectActor::EndOverlap(AActor* TargetActor)
{
	// Early-out for safety.
	if (!TargetActor) return;

	// OVERLAP LIFECYCLE MAPPING:
	// On overlap exit, we apply effects configured for ApplyEndOverlap and remove effects configured for RemoveOnEndOverlap.
	//
	// COMMON USE CASES:
	// - ApplyEndOverlap: Grant-on-exit designs (rare but valid)
	// - RemoveOnEndOverlap: Typical AoE cleanup (fire areas, aura buffs, temporary zones)
	
	ApplyAllGameplayEffects(TargetActor, EGASCoreEffectApplicationPolicy::ApplyEndOverlap);
	RemoveAllGameplayEffects(TargetActor, EGASCoreEffectRemovalPolicy::RemoveOnEndOverlap);
}

void AGASCoreGameplayEffectActor::ApplyAllGameplayEffects(AActor* TargetActor, EGASCoreEffectApplicationPolicy ApplicationPolicy)
{
	// Iterate all rows and apply only those matching this timing.
	// Note: If multiple rows set bDestroyOnEffectApplication, calling Destroy() inside ApplyGameplayEffectToTarget
	// will end processing early. If you need "apply all then destroy", aggregate a flag and Destroy() once after this loop.
	for (const FGASCoreEffectConfig& EffectConfig : GameplayEffects)
	{
		if (EffectConfig.EffectClass && EffectConfig.ApplicationPolicy == ApplicationPolicy)
		{
			ApplyGameplayEffectToTarget(TargetActor, EffectConfig);
		}
	}
}

void AGASCoreGameplayEffectActor::RemoveAllGameplayEffects(AActor* TargetActor, EGASCoreEffectRemovalPolicy RemovalPolicy)
{
	// Iterate all rows and remove only those matching this timing.
	// Removal is per-handle and GC-safe (tracked ASC is weak).
	for (const FGASCoreEffectConfig& EffectConfig : GameplayEffects)
	{
		if (EffectConfig.EffectClass && EffectConfig.RemovalPolicy == RemovalPolicy)
		{
			RemoveGameplayEffectFromTarget(TargetActor, EffectConfig);
		}
	}
}

void AGASCoreGameplayEffectActor::ApplyGameplayEffectToTarget(AActor* TargetActor, const FGASCoreEffectConfig& EffectConfig)
{
	// 1) Resolve the target ASC (supports IAbilitySystemInterface or direct component search).
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	// 2) Sanity-check the effect class (should be set in config).
	checkf(EffectConfig.EffectClass, TEXT("EffectClass is unset on %s"), *GetName());

	// 3) Build an effect context originating from this actor, using the target ASC as the creator.
	//    WHY WE BUILD CONTEXT:
	//    - AddSourceObject(this): Allows consumers (AttributeSet callbacks, gameplay cues) to trace back to this effect actor
	//    - SetEffectCauser(this): Marks this actor as the causer for damage attribution, gameplay logs, and analytics
	//    - Context travels with the effect and is accessible in PostGameplayEffectExecute and other callbacks
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	if (EffectContextHandle.Get())
	{
		EffectContextHandle.Get()->SetEffectCauser(this);
	}

	// 4) Build a spec for this effect at the configured level, with the context provided.
	//    The spec contains all necessary data to apply the effect: magnitudes, duration, tags, etc.
	const FGameplayEffectSpecHandle GameplayEffectSpecHandle =
		TargetASC->MakeOutgoingSpec(EffectConfig.EffectClass, EffectConfig.ActorLevel, EffectContextHandle);
	if (!GameplayEffectSpecHandle.IsValid() || !GameplayEffectSpecHandle.Data.IsValid())
	{
		// Spec creation can fail if class is invalid or gameplay tags/requirements block it.
		return;
	}


	// 5) Apply to the target ASC (apply-to-self on that ASC).
	//    Handle validity:
	//    - Instant: usually invalid (effect executes immediately and ends)
	//    - Duration/Infinite: valid, can be removed/stacked/queried
	const FActiveGameplayEffectHandle ActiveEffectHandle =
		TargetASC->ApplyGameplayEffectSpecToSelf(*GameplayEffectSpecHandle.Data.Get());

	// 6) Track non-instant effects if a removal policy is configured.
	// Periodic effects are either HasDuration or Infinite; both count as non-instant.
	if (ActiveEffectHandle.IsValid() && EffectConfig.RemovalPolicy != EGASCoreEffectRemovalPolicy::DoNotRemove && IsNonInstant(EffectConfig.EffectClass))
	{
		FGASCoreTrackedEffect Track;
		Track.ASC = TargetASC;
		Track.EffectClass = EffectConfig.EffectClass;
		Track.bDestroyOnRemoval = EffectConfig.bDestroyOnEffectRemoval;
		
		// Normalize stacks: <=0 means "remove all"
		Track.StacksToRemove = (EffectConfig.StacksToRemove <= 0) ? -1 : EffectConfig.StacksToRemove;

		// If re-application merges into the same active effect per stacking rules,
		// GAS may return the same handle; TMap::Add will update the value for that key.
		ActiveGameplayEffects.Add(ActiveEffectHandle, Track);
	}

	// 7) Optional: destroy the actor immediately after a successful application (consumables).
	// Caveat: If multiple rows apply in the same overlap tick and each has this flag,
	// the first Destroy() will end further processing. If you need "apply all then destroy",
	// remove Destroy() here and instead aggregate a flag in ApplyAllGameplayEffects.
	if (EffectConfig.bDestroyOnEffectApplication)
	{
		Destroy();
	}
}

void AGASCoreGameplayEffectActor::RemoveGameplayEffectFromTarget(AActor* TargetActor, const FGASCoreEffectConfig& EffectConfig)
{
	// 1) Resolve the target ASC.
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	// 2) Collect matching handles:
	//    - Tracked handle’s ASC is valid AND equals this TargetASC
	//    - Tracked handle’s GE class matches the Config’s EffectClass
	// We do not inspect current active spec Defs here; we use our own tracking to ensure we remove
	// only things this actor applied.
	TArray<FActiveGameplayEffectHandle> HandlesForThisASC;
	HandlesForThisASC.Reserve(ActiveGameplayEffects.Num());
	
	for (TPair<FActiveGameplayEffectHandle, FGASCoreTrackedEffect>& TrackedPair : ActiveGameplayEffects)
	{
		const FGASCoreTrackedEffect& TrackedEffect = TrackedPair.Value;
		
		// Skip if the weak ASC is no longer valid (its owner was GC’d or destroyed)
		if (!TrackedEffect.ASC.IsValid())
		{
			continue;
		}

		// Compare against the current TargetASC (raw pointer) using the weak's Get()
		if (TrackedEffect.ASC.Get() == TargetASC && TrackedEffect.EffectClass == EffectConfig.EffectClass)
		{
			HandlesForThisASC.Add(TrackedPair.Key);
		}
	}

	// 3) Remove stacks/effects for all matching handles and note if anything was removed.
	bool bRemovedAnyStacks = false;
	bool bDestroyAfterRemoval = false; // honor the intent stored per handle

	for (const FActiveGameplayEffectHandle& EffectHandle : HandlesForThisASC)
	{
		const FGASCoreTrackedEffect* TrackedEffect = ActiveGameplayEffects.Find(EffectHandle);
		if (!TrackedEffect) continue;

		// Defensive: ASC could have become invalid between collection and removal
		if (!TrackedEffect->ASC.IsValid()) continue;

		// Attempt removal; returns number of stacks removed (0 if nothing happened).
		// -1 means “remove all stacks” for the matched handle.
		const int32 RemovedCount = TargetASC->RemoveActiveGameplayEffect(EffectHandle, TrackedEffect->StacksToRemove);
		bRemovedAnyStacks |= (RemovedCount != 0);

		// If this particular handle asked for actor destruction on removal, remember it.
		if (RemovedCount != 0 && TrackedEffect->bDestroyOnRemoval)
		{
			bDestroyAfterRemoval = true;
		}
	}

	// 4) Cleanup: erase entries that no longer exist on the ASC OR whose weak ASC is invalid.
	for (const FActiveGameplayEffectHandle& Handle : HandlesForThisASC)
	{
		const FGASCoreTrackedEffect* TrackedEffect = ActiveGameplayEffects.Find(Handle);

		// If ASC invalid, or the handle no longer exists on the ASC, purge it
		const bool bASCInvalid = !TrackedEffect || !TrackedEffect->ASC.IsValid();
		const bool bHandleGone = !bASCInvalid && !TargetASC->GetActiveGameplayEffect(Handle);
		
		if (bASCInvalid || bHandleGone)
		{
			ActiveGameplayEffects.Remove(Handle);
		}
	}

	// 5) Optional: destroy the actor if any handle requested destruction on removal.
	if (bRemovedAnyStacks && bDestroyAfterRemoval)
	{
		Destroy();
	}
}

// ------------ Helper queries on GE classes ------------
// These helpers inspect GameplayEffect class defaults to determine behavior at design time.

EGameplayEffectDurationType AGASCoreGameplayEffectActor::GetDurationPolicyOf(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	if (!EffectClass) return EGameplayEffectDurationType::Instant;

	// Read DurationPolicy from the GE’s Class Default Object (design-time config).
	// Types:
	//   - Instant: Executes once, typically leaves no active handle.
	//   - HasDuration: Active for a time; returns a valid handle.
	//   - Infinite: Active until explicitly removed; returns a valid handle.
	const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
	return EffectCDO ? EffectCDO->DurationPolicy : EGameplayEffectDurationType::Instant;
}

bool AGASCoreGameplayEffectActor::IsPeriodic(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	if (!EffectClass) return false;

	// PERIODIC DETECTION: Check if Period > 0
	// Periodic effects can be HasDuration or Infinite, they execute repeatedly at intervals.
	// Examples: Fire damage every 0.5s, health regeneration every 1.0s, mana drain every 2.0s
	const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
	return EffectCDO && EffectCDO->Period.GetValue() > 0.f;
}

bool AGASCoreGameplayEffectActor::IsInfinite(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	// INFINITE DETECTION: Simple duration policy check
	// Infinite effects last until explicitly removed and require handle tracking.
	// Examples: Permanent buffs, aura effects, fire area damage that persists until leaving
	return GetDurationPolicyOf(EffectClass) == EGameplayEffectDurationType::Infinite;
}

bool AGASCoreGameplayEffectActor::IsNonInstant(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	// Non-instant if DurationPolicy != Instant (covers HasDuration and Infinite; periodic is included).
	return GetDurationPolicyOf(EffectClass) != EGameplayEffectDurationType::Instant;
}
