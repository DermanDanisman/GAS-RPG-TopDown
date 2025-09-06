# Ability Task Montage Integration

Last updated: 2025-01-21

## Overview

Animation montages are a critical component of gameplay abilities, providing visual feedback and timing control for ability execution. The GASCore plugin integrates montage playback with the ability task system to create seamless, interruptible animation sequences that drive ability logic.

This document covers the integration patterns between montages and ability tasks, providing guidance for implementing robust animation-driven abilities.

## Core Concepts

### Montage-Driven Ability Flow

```
Ability Activation → Montage Playback → Animation Events → Ability Logic → Ability End
```

The typical flow involves:
1. **Ability activates** and commits resources
2. **Montage task starts** playing the animation
3. **Animation notify events** trigger ability effects
4. **Montage completion** ends the ability

### Animation Notify Integration

Animation montages contain notify events that trigger specific ability behaviors:

- **AbilityNotify_SpawnProjectile**: Spawns projectiles at precise animation frames
- **AbilityNotify_ApplyDamage**: Applies damage effects for melee attacks
- **AbilityNotify_PlayEffect**: Triggers visual/audio effects
- **AbilityNotify_EndAbility**: Forces ability termination

## Montage Task Implementation Pattern

### Basic Montage Task Structure

```cpp
UCLASS()
class GASCORE_API UAbilityTask_PlayMontageAndWait : public UAbilityTask
{
    GENERATED_BODY()

public:
    // Factory method
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", 
        meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", 
        BlueprintInternalUseOnly = "true"))
    static UAbilityTask_PlayMontageAndWait* PlayMontageAndWait(
        UGameplayAbility* OwningAbility,
        UAnimMontage* MontageToPlay,
        float Rate = 1.f,
        FName StartSection = NAME_None,
        bool bStopWhenAbilityEnds = true);

    // Delegates for different completion states
    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnCompleted;

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnBlendOut;

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnInterrupted;

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnCancelled;

protected:
    virtual void Activate() override;
    virtual void ExternalCancel() override;
    virtual void OnDestroy(bool AbilityEnded) override;

private:
    UPROPERTY()
    UAnimMontage* MontageToPlay;
    
    float PlayRate;
    FName StartSectionName;
    bool bStopWhenAbilityEnds;
    
    // Montage callbacks
    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
```

### Delegate Declaration

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageWaitSimpleDelegate);
```

## Usage in GASCore Abilities

### C++ Implementation Example

```cpp
// In your gameplay ability header
UCLASS()
class GASCORE_API UGASCoreMeleeAbility : public UGASCoreGameplayAbility
{
    GENERATED_BODY()

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    // Montage configuration
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    float PlayRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    FName StartSection = NAME_None;

    // Montage completion handlers
    UFUNCTION()
    void OnAttackMontageCompleted();

    UFUNCTION()
    void OnAttackMontageInterrupted();

    UFUNCTION()
    void OnAttackMontageBlendOut();

    UFUNCTION()
    void OnAttackMontageCancelled();
};

// In your gameplay ability implementation
void UGASCoreMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Start montage playback
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(
            this, AttackMontage, PlayRate, StartSection);

    // Bind completion delegates
    MontageTask->OnCompleted.AddDynamic(this, &UGASCoreMeleeAbility::OnAttackMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UGASCoreMeleeAbility::OnAttackMontageInterrupted);
    MontageTask->OnBlendOut.AddDynamic(this, &UGASCoreMeleeAbility::OnAttackMontageBlendOut);
    MontageTask->OnCancelled.AddDynamic(this, &UGASCoreMeleeAbility::OnAttackMontageCancelled);

    // Activate the task
    MontageTask->ReadyForActivation();
}

void UGASCoreMeleeAbility::OnAttackMontageCompleted()
{
    // Normal completion - end ability
    K2_EndAbility();
}

void UGASCoreMeleeAbility::OnAttackMontageInterrupted()
{
    // Animation was interrupted - cancel ability
    K2_CancelAbility();
}
```

### Blueprint Implementation

In Blueprint graphs, montage tasks provide a clean control flow:

```
[Event ActivateAbility]
        ↓
[Branch: CommitAbility Success?]
    ├─True─→ [Play Montage and Wait]
    │           ├─On Completed─→ [End Ability]
    │           ├─On Interrupted─→ [Cancel Ability]
    │           ├─On Blend Out─→ [Handle Blend Out]
    │           └─On Cancelled─→ [Cancel Ability]
    └─False─→ [End Ability (Failed)]
```

## Animation Notify Events

### Custom Ability Notify Classes

```cpp
UCLASS()
class GASCORE_API UAnimNotify_AbilityEvent : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, 
        UAnimSequenceBase* Animation) override;

    // Event to trigger
    UPROPERTY(EditAnywhere, Category = "Ability")
    FGameplayTag EventTag;

    // Optional payload data
    UPROPERTY(EditAnywhere, Category = "Ability")
    FGameplayEventData EventData;
};

void UAnimNotify_AbilityEvent::Notify(USkeletalMeshComponent* MeshComp, 
    UAnimSequenceBase* Animation)
{
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                ASC->HandleGameplayEvent(EventTag, &EventData);
            }
        }
    }
}
```

### Projectile Spawn Notify

```cpp
UCLASS()
class GASCORE_API UAnimNotify_SpawnProjectile : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, 
        UAnimSequenceBase* Animation) override;

    // Socket name for projectile spawn location
    UPROPERTY(EditAnywhere, Category = "Projectile")
    FName SpawnSocket = "hand_r";

    // Optional projectile class override
    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> ProjectileClassOverride;
};

void UAnimNotify_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, 
    UAnimSequenceBase* Animation)
{
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(Owner))
        {
            // Trigger projectile spawn event
            FGameplayEventData EventData;
            EventData.Instigator = Owner;
            EventData.EventTag = FGameplayTag::RequestGameplayTag("Ability.Event.SpawnProjectile");
            
            if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
            {
                if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
                {
                    ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
                }
            }
        }
    }
}
```

## Advanced Montage Patterns

### Multi-Section Montages

For abilities with multiple phases (e.g., charge, attack, recovery):

```cpp
void UGASCoreChargedAbility::ActivateAbility(/*...*/)
{
    // Start with charge section
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(
            this, ChargeMontage, 1.0f, "Charge");

    MontageTask->OnCompleted.AddDynamic(this, &UGASCoreChargedAbility::OnChargeComplete);
    MontageTask->ReadyForActivation();
}

void UGASCoreChargedAbility::OnChargeComplete()
{
    // Transition to attack section
    UAbilityTask_PlayMontageAndWait* AttackTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(
            this, ChargeMontage, 1.0f, "Attack");

    AttackTask->OnCompleted.AddDynamic(this, &UGASCoreChargedAbility::OnAttackComplete);
    AttackTask->ReadyForActivation();
}
```

### Montage Speed Scaling

```cpp
void UGASCoreRapidAbility::ActivateAbility(/*...*/)
{
    // Calculate attack speed from attributes
    float AttackSpeed = 1.0f;
    if (const UAttributeSet* AttributeSet = GetAbilitySystemComponentFromActorInfo()->GetAttributeSet<UTDAttributeSet>())
    {
        AttackSpeed = AttributeSet->GetAttackSpeed();
    }

    // Scale montage playback speed
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(
            this, AttackMontage, AttackSpeed);

    MontageTask->OnCompleted.AddDynamic(this, &UGASCoreRapidAbility::OnMontageComplete);
    MontageTask->ReadyForActivation();
}
```

### Conditional Montage Selection

```cpp
void UGASCoreComboAbility::ActivateAbility(/*...*/)
{
    // Select montage based on combo state
    UAnimMontage* SelectedMontage = nullptr;
    FName StartSection = NAME_None;

    int32 ComboIndex = GetComboIndex();
    switch (ComboIndex)
    {
        case 0:
            SelectedMontage = ComboMontage;
            StartSection = "Combo1";
            break;
        case 1:
            SelectedMontage = ComboMontage;
            StartSection = "Combo2";
            break;
        case 2:
            SelectedMontage = ComboMontage;
            StartSection = "Combo3";
            break;
        default:
            SelectedMontage = ComboMontage;
            StartSection = "Combo1";
            break;
    }

    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(
            this, SelectedMontage, 1.0f, StartSection);

    MontageTask->OnCompleted.AddDynamic(this, &UGASCoreComboAbility::OnComboComplete);
    MontageTask->ReadyForActivation();
}
```

## Montage Integration with Projectiles

### Timed Projectile Spawning

```cpp
// In projectile ability
void UGASCoreProjectileAbility::ActivateAbility(/*...*/)
{
    // Play casting montage
    UAbilityTask_PlayMontageAndWait* CastTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(
            this, CastMontage, 1.0f);

    CastTask->OnCompleted.AddDynamic(this, &UGASCoreProjectileAbility::OnCastComplete);
    CastTask->ReadyForActivation();

    // Listen for spawn event from animation notify
    UAbilityTask_WaitGameplayEvent* SpawnEventTask = 
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this, FGameplayTag::RequestGameplayTag("Ability.Event.SpawnProjectile"));

    SpawnEventTask->EventReceived.AddDynamic(this, &UGASCoreProjectileAbility::OnSpawnProjectileEvent);
    SpawnEventTask->ReadyForActivation();
}

void UGASCoreProjectileAbility::OnSpawnProjectileEvent(FGameplayEventData Payload)
{
    // Spawn projectile at the precise moment defined by animation
    SpawnActorFromGameplayAbility();
}

void UGASCoreProjectileAbility::OnCastComplete()
{
    // End ability when animation completes
    K2_EndAbility();
}
```

## Error Handling and Edge Cases

### Montage Validation

```cpp
bool UGASCoreMontageAbility::ValidateMontage(UAnimMontage* Montage)
{
    if (!Montage)
    {
        UE_LOG(LogAbilitySystem, Warning, TEXT("Montage is null in ability %s"), *GetName());
        return false;
    }

    // Check if skeletal mesh component exists
    if (USkeletalMeshComponent* SkelMesh = GetActorInfo().SkeletalMeshComponent.Get())
    {
        if (!SkelMesh->GetAnimInstance())
        {
            UE_LOG(LogAbilitySystem, Warning, TEXT("No AnimInstance found for ability %s"), *GetName());
            return false;
        }

        // Validate montage compatibility
        if (!Montage->GetSkeleton()->IsCompatible(SkelMesh->SkeletalMesh->GetSkeleton()))
        {
            UE_LOG(LogAbilitySystem, Error, TEXT("Montage skeleton incompatible in ability %s"), *GetName());
            return false;
        }
    }

    return true;
}
```

### Handling Montage Interruption

```cpp
void UGASCoreMontageAbility::OnMontageInterrupted()
{
    // Check if interruption was due to ability cancellation
    if (IsAbilityEnding())
    {
        // Normal cancellation - just end
        K2_EndAbility();
        return;
    }

    // Unexpected interruption - handle gracefully
    UE_LOG(LogAbilitySystem, Warning, TEXT("Montage unexpectedly interrupted in ability %s"), *GetName());
    
    // Apply any cleanup effects
    ApplyCleanupEffects();
    
    // Cancel the ability
    K2_CancelAbility();
}
```

### Network Considerations

```cpp
void UGASCoreMontageAbility::ActivateAbility(/*...*/)
{
    // Only play montage on server and owning client
    if (HasAuthority(&GetCurrentActivationInfo()) || IsLocallyControlled())
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = 
            UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(this, AttackMontage);

        MontageTask->OnCompleted.AddDynamic(this, &UGASCoreMontageAbility::OnMontageComplete);
        MontageTask->ReadyForActivation();
    }
    else
    {
        // Simulated proxy - just end ability immediately
        K2_EndAbility();
    }
}
```

## Debugging Montage Tasks

### Common Debug Techniques

```cpp
void UGASCoreMontageAbility::ActivateAbility(/*...*/)
{
    UE_LOG(LogAbilitySystem, Log, TEXT("Starting montage %s for ability %s"), 
        *GetNameSafe(AttackMontage), *GetName());

    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(this, AttackMontage);

    // Add debug delegates
    MontageTask->OnCompleted.AddDynamic(this, &UGASCoreMontageAbility::DebugOnComplete);
    MontageTask->OnInterrupted.AddDynamic(this, &UGASCoreMontageAbility::DebugOnInterrupt);
    
    MontageTask->ReadyForActivation();
}

void UGASCoreMontageAbility::DebugOnComplete()
{
    UE_LOG(LogAbilitySystem, Log, TEXT("Montage completed normally for ability %s"), *GetName());
    OnMontageComplete();
}

void UGASCoreMontageAbility::DebugOnInterrupt()
{
    UE_LOG(LogAbilitySystem, Warning, TEXT("Montage interrupted for ability %s"), *GetName());
    OnMontageInterrupt();
}
```

### Blueprint Debug Helpers

Add debug print strings to montage task outputs:

```
[Play Montage and Wait]
    ├─On Completed─→ [Print String: "Montage Completed"] → [End Ability]
    ├─On Interrupted─→ [Print String: "Montage Interrupted"] → [Cancel Ability]
    └─On Cancelled─→ [Print String: "Montage Cancelled"] → [Cancel Ability]
```

## Performance Considerations

### Montage Pooling

```cpp
class GASCORE_API UMontagePool : public UObject
{
public:
    static UMontagePool* Get();
    
    UAbilityTask_PlayMontageAndWait* GetPooledMontageTask(UGameplayAbility* Ability);
    void ReturnMontageTask(UAbilityTask_PlayMontageAndWait* Task);

private:
    TArray<UAbilityTask_PlayMontageAndWait*> AvailableTasks;
    TArray<UAbilityTask_PlayMontageAndWait*> ActiveTasks;
};
```

### Memory Management

```cpp
void UGASCoreMontageAbility::OnDestroy(bool AbilityIsEnding)
{
    // Clean up any cached montage references
    CurrentMontageTask = nullptr;
    
    Super::OnDestroy(AbilityIsEnding);
}
```

## Related Documentation

- [Gameplay Ability Tasks](gameplay-ability-tasks.md)
- [Projectile Abilities Pattern](projectile-abilities.md)
- [Combat Interface](combat-interface.md)
- [Gameplay Abilities Concepts and Practice](../guides/GameplayAbilities_Concepts_And_Practice.md)

## See Also

- [Unreal Engine Animation Montage Documentation](https://docs.unrealengine.com/5.3/en-US/animation-montage-in-unreal-engine/)
- [Unreal Engine Animation Notifications](https://docs.unrealengine.com/5.3/en-US/animation-notifications-in-unreal-engine/)