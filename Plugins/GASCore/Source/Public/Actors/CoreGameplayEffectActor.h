// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Summary:
//   A versatile world actor that applies and manages Gameplay Effects to overlapping targets.
//   It supports Instant, Duration, Periodic, and Infinite effects, with configurable
//   application/removal policies, level/scaling, and optional actor self-destruction.
//
// Design:
//   - Data-driven: designers configure an array of FCoreEffectConfig entries.
//   - Overlap-driven: call OnOverlap/EndOverlap from collision events (or manually).
//   - Tracking: Infinite effects applied by this actor are tracked per-target ASC via handles
//               to enable precise removal later.
//   - UI/Debug: emits verbose logs describing configuration and operations.
//
// Usage (typical Blueprint workflow):
//   - Place ACoreGameplayEffectActor in the level and add desired collision component(s).
//   - Configure GameplayEffects array (EffectClass, ApplicationPolicy, RemovalPolicy, etc.).
//   - Bind the collision's Begin/End overlap to OnOverlap/EndOverlap respectively.
//   - For Infinite effects, ensure each entry uses a distinct GameplayEffect class.
//
// Networking note:
//   - Applying/removing gameplay effects is typically a server-authoritative action.
//     Ensure your overlap events run on the server or route calls appropriately (e.g., via server RPC).
//
// Related helpers:
//   - AbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target)
//   - UAbilitySystemComponent::MakeEffectContext / MakeOutgoingSpec / ApplyGameplayEffectSpecToSelf
//   - UAbilitySystemComponent::RemoveActiveGameplayEffect
//
// Implementation: see Actors/CoreGameplayEffectActor.cpp

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "CoreGameplayEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

// -----------------------------------------------------------------------------
// Effect timing policies
// -----------------------------------------------------------------------------

/**
 * When to apply the effect relative to overlap events.
 */
UENUM(BlueprintType)
enum class ECoreEffectApplicationPolicy : uint8
{
	/** Apply the effect when an actor begins overlap with this actor */
	ApplyOnOverlap UMETA(DisplayName = "Apply On Overlap"),

	/** Apply the effect when an actor ends overlap with this actor */
	ApplyEndOverlap UMETA(DisplayName = "Apply On End Overlap"),

	/** Do not apply the effect automatically (manual application only) */
	DoNotApply UMETA(DisplayName = "Do Not Apply")
};

/**
 * When to remove the effect relative to overlap events.
 * Only meaningful for non-instant effects (HasDuration/Infinite, including Periodic).
 */
UENUM(BlueprintType)
enum class ECoreEffectRemovalPolicy : uint8
{
	/** Remove the effect when an actor begins overlap with this actor */
	RemoveOnOverlap UMETA(DisplayName = "Remove On Overlap"),

	/** Remove the effect when an actor ends overlap with this actor */
	RemoveOnEndOverlap UMETA(DisplayName = "Remove On End Overlap"),

	/** Do not remove the effect automatically (manual removal only) */
	DoNotRemove UMETA(DisplayName = "Do Not Remove")
};

// -----------------------------------------------------------------------------
// Effect configuration (one row per effect entry)
// -----------------------------------------------------------------------------

/**
 * Configuration for a single Gameplay Effect entry.
 * Contains all data necessary to apply and manage a Gameplay Effect instance.
 */
USTRUCT(BlueprintType)
struct FCoreEffectConfig
{
	GENERATED_BODY()

	/** The Gameplay Effect class to apply. This drives duration policy, periodicity, and stacking behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

	/** When this effect should be applied relative to overlap events. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	ECoreEffectApplicationPolicy ApplicationPolicy;

	/**
	 * When this effect should be removed relative to overlap events.
	 * Applies only to non-instant effects (HasDuration or Infinite; Periodic counts as non-instant).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	ECoreEffectRemovalPolicy RemovalPolicy;

	/** Destroy this actor immediately after a successful application of this effect (useful for consumables). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	bool bDestroyOnEffectApplication;

	/** Destroy this actor immediately after this effect is removed (e.g., on leaving an AoE fire). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect")
	bool bDestroyOnEffectRemoval;

	/**
	 * Effect level used when creating the outgoing spec.
	 * Drives scalable magnitudes/tables defined in the GE (if any).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect", meta = (ClampMin = "1.0"))
	float ActorLevel;

	/**
	 * How many stacks to remove when RemovalPolicy triggers:
	 * -1 = remove all stacks for the matched handle
	 *  1 = remove a single stack (classic: leave one fire area, remove one stack)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Effect", meta = (ClampMin = "-1"))
	int32 StacksToRemove;

	FCoreEffectConfig()
	{
		EffectClass = nullptr;
		ApplicationPolicy = ECoreEffectApplicationPolicy::DoNotApply;
		RemovalPolicy = ECoreEffectRemovalPolicy::DoNotRemove;
		bDestroyOnEffectApplication = false;
		bDestroyOnEffectRemoval = false;
		ActorLevel = 1.0f;
		StacksToRemove = -1;
	}
};

// -----------------------------------------------------------------------------
// Gameplay Effect Actor
// -----------------------------------------------------------------------------

/**
 * ACoreGameplayEffectActor
 *
 * A world actor that applies and manages Gameplay Effects on overlapping targets.
 * - Supports Instant, Duration, Periodic, and Infinite effects
 * - Tracks Infinite effect handles per target ASC for precise removal
 * - Blueprint-callable OnOverlap/EndOverlap entry points (bind these to collision events)
 */
UCLASS()
class GASCORE_API ACoreGameplayEffectActor : public AActor
{
	GENERATED_BODY()

public:
	ACoreGameplayEffectActor();

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
	UFUNCTION(BlueprintCallable, Category = "GASCore|Gameplay Effect Actor")
	void OnOverlap(AActor* TargetActor);

	/**
	 * Called when an actor ends overlap with this effect actor.
	 * Applies/removes effects based on each entry's ApplicationPolicy/RemovalPolicy.
	 *
	 * @param TargetActor The actor that ended overlapping with this actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "GASCore|Gameplay Effect Actor")
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASCore|Gameplay Effects", meta = (TitleProperty = "EffectClass"))
	TArray<FCoreEffectConfig> GameplayEffects;

private:
	// -----------------------------------------------------------------------
	// COMPONENTS
	// -----------------------------------------------------------------------

	/** Root scene component so designers can attach any collision/visual components in BP. */
	UPROPERTY(VisibleAnywhere, Category = "GASCore|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	// -----------------------------------------------------------------------
	// INFINITE EFFECT TRACKING
	// -----------------------------------------------------------------------

	/**
	 * Maps active Infinite effect handles to their target ASCs.
	 * Enables precise removal of effects applied specifically by THIS actor instance.
	 * (Handles for Instant effects are typically invalid and are not tracked.)
	 */
	UPROPERTY()
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveInfiniteEffects;

	// -----------------------------------------------------------------------
	// CORE OPERATIONS
	// -----------------------------------------------------------------------

	/** Apply all effects whose ApplicationPolicy matches the given timing. */
	void ApplyAllGameplayEffects(AActor* TargetActor, ECoreEffectApplicationPolicy ApplicationPolicy);

	/** Remove all effects whose RemovalPolicy matches the given timing. */
	void RemoveAllGameplayEffects(AActor* TargetActor, ECoreEffectRemovalPolicy RemovalPolicy);

	/** Build context/spec and apply a single configured effect to the target. */
	void ApplyGameplayEffectToTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig);

	/** Remove stacks/effects previously applied by this actor that match the config. */
	void RemoveGameplayEffectFromTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig);

	// -----------------------------------------------------------------------
	// HELPERS (inspect GE class characteristics)
	// -----------------------------------------------------------------------

	/** Returns the duration policy (Instant/HasDuration/Infinite) of a GE class. */
	static EGameplayEffectDurationType GetDurationPolicyOf(const TSubclassOf<UGameplayEffect>& EffectClass);

	/** Returns true if the GE class has a Period > 0 (i.e., executes periodically). */
	static bool IsPeriodic(const TSubclassOf<UGameplayEffect>& EffectClass);

	/** Returns true if the GE class has Infinite duration. */
	static bool IsInfinite(const TSubclassOf<UGameplayEffect>& EffectClass);
};