// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Summary (header):
// - Data-driven GameplayEffect application/removal controlled by FCoreEffectConfig entries.
// - Tracks non-instant effects (HasDuration or Infinite; Periodic is covered) via handles for precise removal.
// - Uses TWeakObjectPtr for GC-safe ASC tracking.
// - See CoreGameplayEffect.cpp for implementation (recommend renaming that file to a .cpp).
//
// Design:
//   - Data-driven: designers configure an array of FCoreEffectConfig entries.
//   - Overlap-driven: call OnOverlap/EndOverlap from collision events (or manually).
//   - Tracking: Non-instant effects applied by this actor are tracked per-target ASC via handles
//               to enable precise removal later (periodic effects are covered because they’re non-instant).
//
// Usage (typical Blueprint workflow):
//   - Place AGASCoreGameplayEffectActor in the level and add desired collision component(s).
//   - Configure GameplayEffects array (EffectClass, ApplicationPolicy, RemovalPolicy, etc.).
//   - Bind the collision's Begin/End overlap to OnOverlap/EndOverlap respectively.
//   - If you configure bDestroyOnEffectApplication on multiple entries, consider deferring destruction
//     until all entries are processed to avoid early-exit (see note in implementation).
//
// Networking note:
//   - Applying/removing gameplay effects is typically server-authoritative.
//     Ensure overlap events run on the server or route via server RPCs (HasAuthority()).
//
// Implementation: see Actors/CoreGameplayEffectActor.cpp

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "GASCoreGameplayEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

// -----------------------------------------------------------------------------
// Effect timing policies
// -----------------------------------------------------------------------------

/**
 * When to apply the effect relative to overlap events.
 */
UENUM(BlueprintType)
enum class EGASCoreEffectApplicationPolicy : uint8
{
	ApplyOnOverlap   UMETA(DisplayName = "Apply On Overlap"),    // Fire during OnOverlap
	ApplyEndOverlap  UMETA(DisplayName = "Apply On End Overlap"),// Fire during EndOverlap
	DoNotApply       UMETA(DisplayName = "Do Not Apply")         // Manual/other control only
};

/**
 * When to remove the effect relative to overlap events.
 * Only meaningful for non-instant effects (HasDuration/Infinite; Periodic is non-instant).
 */
UENUM(BlueprintType)
enum class EGASCoreEffectRemovalPolicy : uint8
{
	RemoveOnOverlap     UMETA(DisplayName = "Remove On Overlap"),    // Remove during OnOverlap
	RemoveOnEndOverlap  UMETA(DisplayName = "Remove On End Overlap"),// Remove during EndOverlap
	DoNotRemove         UMETA(DisplayName = "Do Not Remove")         // Never auto-remove
};

// -----------------------------------------------------------------------------
// Effect configuration (one row per effect entry)
// -----------------------------------------------------------------------------

/**
 * One GameplayEffect configuration row.
 * Designers add multiple rows; each row applies/removes independently.
 */
USTRUCT(BlueprintType)
struct FGASCoreEffectConfig
{
	GENERATED_BODY()

	/** GE class to apply. Defines duration policy, periodic tick, stacking rules, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

	/** When this effect should be applied relative to overlaps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	EGASCoreEffectApplicationPolicy ApplicationPolicy;

	/** When this effect should be removed relative to overlaps (non-instant effects only). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	EGASCoreEffectRemovalPolicy RemovalPolicy;

	/** Destroy this actor immediately after applying this effect (consumables, one-shots). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	bool bDestroyOnEffectApplication;

	/** Destroy this actor immediately after this effect is removed (e.g., leave an AoE). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	bool bDestroyOnEffectRemoval;

	/** Level used for the outgoing spec (scales magnitudes, tables, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect", meta = (ClampMin = "1.0"))
	float ActorLevel;

	/**
	 * How many stacks to remove when RemovalPolicy triggers:
	 * -1 = remove all stacks on the matched handle (recommended for “remove all”)
	 *  1 = remove a single stack (common for stacking auras/areas)
	 * >1 = remove exactly that many stacks
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect", meta = (ClampMin = "-1"))
	int32 StacksToRemove;

	FGASCoreEffectConfig()
	{
		EffectClass = nullptr;
		ApplicationPolicy = EGASCoreEffectApplicationPolicy::DoNotApply;
		RemovalPolicy = EGASCoreEffectRemovalPolicy::DoNotRemove;
		bDestroyOnEffectApplication = false;
		bDestroyOnEffectRemoval = false;
		ActorLevel = 1.0f;
		StacksToRemove = -1; // -1 == remove all
	}
};

/**
 * Per-handle tracking info for non-instant effects applied by this actor
 * that we intend to remove later based on configured RemovalPolicy.
 *
 * GC-safety:
 * - ASC is tracked with TWeakObjectPtr so if the component or its owner is destroyed
 *   (level stream out, death, cleanup), the pointer becomes null instead of dangling.
 * - Always check IsValid() before using ASC.
 *
 * Matching:
 * - We store the exact GE class and a per-handle “destroy on removal” intent as of application time,
 *   so multiple config rows using the same GE class with different intentions won’t conflict.
 */
USTRUCT()
struct FGASCoreTrackedEffect
{
	GENERATED_BODY()

	/** Weak reference to the target ASC (GC-safe). Use IsValid() and Get() before use. */
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	/** The exact GE class used. Used to match during removal requests. */
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;

	/** Stacks to remove when removal triggers. -1 means “remove all stacks”. */
	int32 StacksToRemove = -1;

	/** If true, destroy this actor after this specific handle is removed. */
	bool bDestroyOnRemoval = false;
};

/**
 * AGASCoreGameplayEffectActor
 *
 * A world actor that applies and manages Gameplay Effects on overlapping targets.
 * - Supports Instant, Duration, Periodic, and Infinite effects
 * - Tracks Infinite effect handles per target ASC for precise removal
 * - Blueprint-callable OnOverlap/EndOverlap entry points (bind these to collision events)
 */
UCLASS()
class GASCORE_API AGASCoreGameplayEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AGASCoreGameplayEffectActor();

protected:
	virtual void BeginPlay() override;

public:
	// -----------------------------------------------------------------------
	// OVERLAP EVENT HANDLERS (call from collision events or manually)
	// -----------------------------------------------------------------------

	/**
	 * Called when an actor overlaps with this effect actor.
	 * Applies/removes effects based on each entry's ApplicationPolicy/RemovalPolicy.
	 *
	 * @param TargetActor The actor that began overlapping with this actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "GASCore|Gameplay Effect Actor|Functions")
	void OnOverlap(AActor* TargetActor);

	/**
	 * Called when an actor ends overlap with this effect actor.
	 * Applies/removes effects based on each entry's ApplicationPolicy/RemovalPolicy.
	 *
	 * @param TargetActor The actor that ended overlapping with this actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "GASCore|Gameplay Effect |Functions")
	void EndOverlap(AActor* TargetActor);

protected:
	// -----------------------------------------------------------------------
	// EFFECT CONFIGURATIONS
	// -----------------------------------------------------------------------

	/**
	 * All Gameplay Effects that can be applied by this actor.
	 * Notes:
	 * - Instant effects: applied immediately; no handle tracking (handle typically invalid).
	 * - HasDuration/Periodic: temporary effects; may tick periodically.
	 * - Infinite: lasts until explicitly removed; tracked via handle for precise removal.
	 *
	 * IMPORTANT:
	 * - For multiple Infinite effects, use distinct GE classes to avoid ambiguous tracking.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Gameplay Effect Actor|Gameplay Effects", meta = (TitleProperty = "EffectClass"))
	TArray<FGASCoreEffectConfig> GameplayEffects;

private:
	// -----------------------------------------------------------------------
	// COMPONENTS
	// -----------------------------------------------------------------------

	/** Root scene component so designers can attach any collision/visual components in BP. */
	UPROPERTY(VisibleAnywhere, Category = "GASCore|Gameplay Effect Actor|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	// -----------------------------------------------------------------------
	// TRACKED EFFECTS (Duration and Infinite)
	// -----------------------------------------------------------------------

	/**
	 * Handles of non-instant effects applied by this actor, for precise removal.
	 * Key: active handle returned by ApplyGameplayEffectSpecToSelf.
	 * Val: per-handle tracking info (weak ASC, class, stacks, destroy-on-removal).
	 */
	UPROPERTY()
	TMap<FActiveGameplayEffectHandle, FGASCoreTrackedEffect> ActiveGameplayEffects;

	// -----------------------------------------------------------------------
	// CORE OPERATIONS
	// -----------------------------------------------------------------------

	/** Apply all effects whose ApplicationPolicy matches the given timing. */
	void ApplyAllGameplayEffects(AActor* TargetActor, EGASCoreEffectApplicationPolicy ApplicationPolicy);

	/** Remove all effects whose RemovalPolicy matches the given timing. */
	void RemoveAllGameplayEffects(AActor* TargetActor, EGASCoreEffectRemovalPolicy RemovalPolicy);

	/** Build context/spec and apply a single configured effect to the target. */
	void ApplyGameplayEffectToTarget(AActor* TargetActor, const FGASCoreEffectConfig& EffectConfig);

	/** Remove stacks/effects previously applied by this actor that match the config. */
	void RemoveGameplayEffectFromTarget(AActor* TargetActor, const FGASCoreEffectConfig& EffectConfig);

	// -----------------------------------------------------------------------
	// HELPERS (inspect GE class characteristics)
	// -----------------------------------------------------------------------

	/** Returns the duration policy (Instant/HasDuration/Infinite) of a GE class. */
	static EGameplayEffectDurationType GetDurationPolicyOf(const TSubclassOf<UGameplayEffect>& EffectClass);

	/** Returns true if the GE class has a Period > 0 (i.e., executes periodically). */
	static bool IsPeriodic(const TSubclassOf<UGameplayEffect>& EffectClass);

	/** Returns true if the GE class has Infinite duration. */
	static bool IsInfinite(const TSubclassOf<UGameplayEffect>& EffectClass);

	/** Returns true if non-instant (HasDuration or Infinite). */
	static bool IsNonInstant(const TSubclassOf<UGameplayEffect>& EffectClass);
};