// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "CoreGameplayEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

// A logging channel for this actor
DECLARE_LOG_CATEGORY_EXTERN(LogCoreGEA, Log, All);

/**
 * When to apply the effect
 */
UENUM(BlueprintType)
enum class ECoreEffectApplicationPolicy : uint8
{
    /** Apply the effect when an actor begins overlap with this actor */
    ApplyOnOverlap,
    
    /** Apply the effect when an actor ends overlap with this actor */
    ApplyEndOverlap,
    
    /** Do not apply the effect automatically */
    DoNotApply
};

/**
 * When to remove the effect
 */
UENUM(BlueprintType)
enum class ECoreEffectRemovalPolicy : uint8
{
    /** Remove the effect when an actor begins to overlap with this actor */
    RemoveOnOverlap,
    
    /** Remove the effect when an actor ends overlap with this actor */
    RemoveOnEndOverlap,
    
    /** Do not remove the effect automatically */
    DoNotRemove
};

/**
 * The Configuration structure for a single gameplay effect
 * Contains all necessary data to apply and manage a gameplay effect
 */
USTRUCT(BlueprintType)
struct FCoreEffectConfig
{
    GENERATED_BODY()

    /** The Gameplay Effect class to apply */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    TSubclassOf<UGameplayEffect> EffectClass;

    /** When to apply this effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    ECoreEffectApplicationPolicy ApplicationPolicy;

    /** When to remove this effect (only applies to Duration/Periodic/Infinite effects) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    ECoreEffectRemovalPolicy RemovalPolicy;

    /** Should this actor be destroyed when the effect is applied? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    bool bDestroyOnEffectApplication;

    /** Should this actor be destroyed when the effect is removed? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    bool bDestroyOnEffectRemoval;

    /** Effect level to apply (drives scalable magnitudes/tables if used) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "1.0"))
    float ActorLevel;

    /**
     * How many stacks to remove when we remove this effect by policy:
     * -1 = remove all stacks for the matched handle
     *  1 = remove a single stack (classic “leave one fire area, remove one stack”)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "-1"))
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

/**
 * A versatile world actor that applies and manages Gameplay Effects.
 * - Supports Instant, Duration, Periodic, and Infinite effects
 * - Tracks Infinite effects by handle per overlapping ASC for precise removal
 * - Blueprint-callable OnOverlap/EndOverlap entry points
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
    //-----------------------------------------------------------------------
    // OVERLAP EVENT HANDLERS
    //-----------------------------------------------------------------------

    /**
     * Called when an actor overlaps with this effect actor
     * Handles application and removal based on configured policies
     *
     * @param TargetActor - The actor that began overlapping with this actor
     */
    UFUNCTION(BlueprintCallable, Category = "Gameplay Effect Actor")
    void OnOverlap(AActor* TargetActor);

    /**
     * Called when an actor ends overlap with this effect actor
     * Handles application and removal based on configured policies
     *
     * @param TargetActor - The actor that ended overlapping with this actor
     */
    UFUNCTION(BlueprintCallable, Category = "Gameplay Effect Actor")
    void EndOverlap(AActor* TargetActor);

protected:
    //-----------------------------------------------------------------------
    // EFFECT CONFIGURATIONS
    //-----------------------------------------------------------------------

    /** 
     * All gameplay effects that can be applied by this actor
     * Effects are automatically categorized based on their duration policy:
     * - Instant: Applied immediately with no duration
     * - Duration: Lasts for a specified time
     * - Periodic: Executes repeatedly over time
     * - Infinite: Lasts until manually removed
     * 
     * NOTE: When configuring multiple infinite effects, make sure each uses a DIFFERENT
     * GameplayEffect class to ensure proper tracking for removal.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Effects", meta = (TitleProperty = "EffectClass"))
    TArray<FCoreEffectConfig> GameplayEffects;

private:
    // Root to let designers add any shape/components they wish in Blueprint
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USceneComponent> DefaultSceneRoot;

    //-----------------------------------------------------------------------
    // INFINITE EFFECT TRACKING
    //-----------------------------------------------------------------------

    /**
     * Maps active infinite effect handles to their target ability system components
     * This enables precise removal of effects applied by THIS specific actor
     * Critical for handling overlapping effect areas correctly
     */
    UPROPERTY()
    TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveInfiniteEffects;
    
    //-----------------------------------------------------------------------
    // CoreGameplay EFFECT OPERATIONS
    //-----------------------------------------------------------------------
    
    // Internals
    void ApplyAllGameplayEffects(AActor* TargetActor, ECoreEffectApplicationPolicy ApplicationPolicy);
    void RemoveAllGameplayEffects(AActor* TargetActor, ECoreEffectRemovalPolicy RemovalPolicy);

    void ApplyGameplayEffectToTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig);
    void RemoveGameplayEffectFromTarget(AActor* TargetActor, const FCoreEffectConfig& EffectConfig);

    // Helpers
    static EGameplayEffectDurationType GetDurationPolicyOf(const TSubclassOf<UGameplayEffect>& EffectClass);
    static bool IsPeriodic(const TSubclassOf<UGameplayEffect>& EffectClass);
    static bool IsInfinite(const TSubclassOf<UGameplayEffect>& EffectClass);
};