// © 2025 Heathrow (Derman). All rights reserved.

#include "Actors/CoreGameplayEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

// Logging category for this actor (see .h where it's declared with DECLARE_LOG_CATEGORY_EXTERN)
DEFINE_LOG_CATEGORY(LogCoreGEA);

namespace
{
    // Pretty-print the GE duration policy
    const TCHAR* ToStrDurationPolicy(EGameplayEffectDurationType Policy)
    {
        switch (Policy)
        {
        case EGameplayEffectDurationType::Instant:     return TEXT("Instant");
        case EGameplayEffectDurationType::HasDuration: return TEXT("HasDuration");
        case EGameplayEffectDurationType::Infinite:    return TEXT("Infinite");
        default:                                       return TEXT("Unknown");
        }
    }

    // Compact description of a built spec for logging
    FString DescribeSpec(const FGameplayEffectSpecHandle& SpecHandle)
    {
        if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
        {
            return TEXT("Spec: <invalid>");
        }

        const FGameplayEffectSpec& Spec = *SpecHandle.Data.Get();
        return FString::Printf(
            TEXT("Spec: Def=%s Level=%.2f Duration=%.3f Period=%.3f Stack=%d"),
            *GetNameSafe(Spec.Def),
            Spec.GetLevel(),
            Spec.GetDuration(),
            Spec.GetPeriod(),
            Spec.GetStackCount()
        );
    }
}

ACoreGameplayEffectActor::ACoreGameplayEffectActor()
{
    // Give designers a neutral root so they can add any collision/visuals in BP
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
    SetRootComponent(DefaultSceneRoot);
}

void ACoreGameplayEffectActor::BeginPlay()
{
    Super::BeginPlay();

    // Warn if the same Infinite GE class was configured multiple times in this single actor.
    // Using duplicates makes handle-based tracking/removal ambiguous.
    TSet<TSubclassOf<UGameplayEffect>> SeenInfiniteClasses;
    for (const FCoreEffectConfig& Config : GameplayEffects)
    {
        if (IsInfinite(Config.EffectClass))
        {
            if (SeenInfiniteClasses.Contains(Config.EffectClass))
            {
                UE_LOG(
                    LogCoreGEA,
                    Warning,
                    TEXT("[%s] Duplicate Infinite GE class configured: %s"),
                    *GetName(),
                    *GetNameSafe(Config.EffectClass)
                );
            }
            else
            {
                SeenInfiniteClasses.Add(Config.EffectClass);
            }
        }
    }

    // Configuration summary for quick validation in logs
    for (const FCoreEffectConfig& Config : GameplayEffects)
    {
        if (!Config.EffectClass) continue;

        const EGameplayEffectDurationType DurationPolicy = GetDurationPolicyOf(Config.EffectClass);
        const bool bIsPeriodic = IsPeriodic(Config.EffectClass);
        const TCHAR* TypeStr =
            (DurationPolicy == EGameplayEffectDurationType::Instant)  ? TEXT("Instant")  :
            (DurationPolicy == EGameplayEffectDurationType::Infinite) ? TEXT("Infinite") :
            (bIsPeriodic ? TEXT("Periodic") : TEXT("Duration"));

        UE_LOG(
            LogCoreGEA,
            Verbose,
            TEXT("[%s] GE=%s Type=%s Apply=%d Remove=%d Level=%.1f StacksToRemove=%d"),
            *GetName(),
            *GetNameSafe(Config.EffectClass),
            TypeStr,
            int32(Config.ApplicationPolicy),
            int32(Config.RemovalPolicy),
            Config.ActorLevel,
            Config.StacksToRemove
        );
    }
}

void ACoreGameplayEffectActor::OnOverlap(AActor* TargetActor)
{
    if (!TargetActor) return;

    // On overlap, apply what should apply now, and also remove anything configured to
    // be removed when a new overlap begins (rare, but supported).
    ApplyAllGameplayEffects(TargetActor, ECoreEffectApplicationPolicy::ApplyOnOverlap);
    RemoveAllGameplayEffects(TargetActor, ECoreEffectRemovalPolicy::RemoveOnOverlap);
}

void ACoreGameplayEffectActor::EndOverlap(AActor* TargetActor)
{
    if (!TargetActor) return;

    // On end overlap, apply what is configured for that timing
    // (some designs' grant on exit), then remove what should be removed on exit.
    ApplyAllGameplayEffects(TargetActor, ECoreEffectApplicationPolicy::ApplyEndOverlap);
    RemoveAllGameplayEffects(TargetActor, ECoreEffectRemovalPolicy::RemoveOnEndOverlap);
}

void ACoreGameplayEffectActor::ApplyAllGameplayEffects(AActor* TargetActor, ECoreEffectApplicationPolicy ApplicationPolicy)
{
    // Iterate once through the configured effects and apply only those whose ApplicationPolicy matches this event.
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
    // Iterate once through the configured effects and remove only those whose RemovalPolicy matches this event.
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
    // 1) Resolve the target ASC (supports IAbilitySystemInterface or component lookup)
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC)
    {
        UE_LOG(
            LogCoreGEA,
            Verbose,
            TEXT("[%s] Apply aborted: %s has no ASC"),
            *GetName(),
            *GetNameSafe(TargetActor)
        );
        return;
    }

    // 2) Validate the effect class (we expect it to be set in the config array)
    checkf(EffectConfig.EffectClass, TEXT("EffectClass is unset on %s"), *GetName());

    // 3) Build an effect context originating from this actor, using the target ASC as the maker.
    //    Using the target's ASC here is safe for "world effect actors" that don't own an ASC.
    FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
    EffectContextHandle.AddSourceObject(this);
    if (EffectContextHandle.Get())
    {
        EffectContextHandle.Get()->SetEffectCauser(this);
    }

    // 4) Build a spec for this effect at the configured level, with the context we just created.
    const FGameplayEffectSpecHandle GameplayEffectSpecHandle =
        TargetASC->MakeOutgoingSpec(EffectConfig.EffectClass, EffectConfig.ActorLevel, EffectContextHandle);

    if (!GameplayEffectSpecHandle.IsValid() || !GameplayEffectSpecHandle.Data.IsValid() || GameplayEffectSpecHandle.Data->Def == nullptr)
    {
        UE_LOG(
            LogCoreGEA,
            Warning,
            TEXT("[%s] Invalid Spec for %s -> %s"),
            *GetName(),
            *GetNameSafe(EffectConfig.EffectClass),
            *GetNameSafe(TargetActor)
        );
        return;
    }

    // 5) Apply the spec to the target ASC (Apply-to-Self pattern).
    //    Note: For Instant effects the returned handle is INDEX_NONE (-1), which is expected.
    const FActiveGameplayEffectHandle ActiveEffectHandle =
        TargetASC->ApplyGameplayEffectSpecToSelf(*GameplayEffectSpecHandle.Data.Get());

    UE_LOG(
        LogCoreGEA,
        VeryVerbose,
        TEXT("[%s] Applied %s to %s | %s | Handle=%s"),
        *GetName(),
        *GetNameSafe(EffectConfig.EffectClass),
        *GetNameSafe(TargetActor),
        *DescribeSpec(GameplayEffectSpecHandle),
        *ActiveEffectHandle.ToString()
    );

    // 6) If this is an Infinite effect and we plan to remove it later, track the handle per target ASC.
    if (IsInfinite(EffectConfig.EffectClass)
        && EffectConfig.RemovalPolicy != ECoreEffectRemovalPolicy::DoNotRemove
        && ActiveEffectHandle.IsValid())
    {
        ActiveInfiniteEffects.Add(ActiveEffectHandle, TargetASC);
    }

    // 7) Optional: destroy the effect actor right after a successful application
    if (EffectConfig.bDestroyOnEffectApplication)
    {
        Destroy();
    }
}

void ACoreGameplayEffectActor::RemoveGameplayEffectFromTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig)
{
    // 1) Resolve the target ASC
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC) return;

    // 2) Collect only the handles that:
    //    - We previously recorded in ActiveInfiniteEffects
    //    - Belong to THIS TargetASC
    //    - And whose active spec.Def class matches EffectConfig.EffectClass
    //    We also collect stale handles (no longer active) to clean them up afterward.
    TArray<FActiveGameplayEffectHandle> HandlesForThisASC;
    for (const TPair<FActiveGameplayEffectHandle, UAbilitySystemComponent*>& TrackedPair : ActiveInfiniteEffects)
    {
        const FActiveGameplayEffectHandle TrackedHandle = TrackedPair.Key;
        UAbilitySystemComponent* ASCForHandle = TrackedPair.Value;

        if (ASCForHandle != TargetASC)
        {
            continue; // different target ASC
        }

        const FActiveGameplayEffect* ActiveEffect = TargetASC->GetActiveGameplayEffect(TrackedHandle);
        if (!ActiveEffect)
        {
            // Stale entry, still add for cleanup
            HandlesForThisASC.Add(TrackedHandle);
            continue;
        }

        if (ActiveEffect->Spec.Def && ActiveEffect->Spec.Def->GetClass() == EffectConfig.EffectClass)
        {
            HandlesForThisASC.Add(TrackedHandle);
        }
    }

    // 3) Remove stacks/effects for all matching handles and note if we removed anything
    bool bRemovedAnyStacks = false;

    for (const FActiveGameplayEffectHandle& Handle : HandlesForThisASC)
    {
        const FActiveGameplayEffect* ExistingActiveEffect = TargetASC->GetActiveGameplayEffect(Handle);
        if (!ExistingActiveEffect)
        {
            // Already stale, skip removal; cleanup will erase it below
            continue;
        }

        // RemoveActiveGameplayEffect returns number of stacks removed (or 0 if nothing removed)
        // StacksToRemove: -1 means remove all stacks; 1 means remove a single stack.
        const int32 RemovedCount = TargetASC->RemoveActiveGameplayEffect(Handle, EffectConfig.StacksToRemove);
        bRemovedAnyStacks |= (RemovedCount != 0);

        UE_LOG(
            LogCoreGEA,
            Verbose,
            TEXT("[%s] Remove %s on %s: Handle=%s StacksRemoved=%d"),
            *GetName(),
            *GetNameSafe(EffectConfig.EffectClass),
            *GetNameSafe(TargetActor),
            *Handle.ToString(),
            RemovedCount
        );
    }

    // 4) Cleanup: erase handles that no longer exist on the ASC
    for (const FActiveGameplayEffectHandle& Handle : HandlesForThisASC)
    {
        if (!TargetASC->GetActiveGameplayEffect(Handle))
        {
            ActiveInfiniteEffects.Remove(Handle);
        }
    }

    // 5) Optional: destroy the effect actor after a successful removal
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