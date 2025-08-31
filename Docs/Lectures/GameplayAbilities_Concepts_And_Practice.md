# Gameplay Abilities in GAS — Concepts and Practice (Deep Dive)

This comprehensive guide covers the fundamentals of Gameplay Abilities in Unreal Engine's Gameplay Ability System (GAS), with specific references to the TD RPG TopDown project implementation. Whether you're working in C++ or Blueprints, this document will provide you with the deep understanding needed to effectively create and manage gameplay abilities.

## Table of Contents

- [What is a Gameplay Ability?](#what-is-a-gameplay-ability)
- [Activation Lifecycle](#activation-lifecycle)
- [Replication & Prediction](#replication--prediction)
- [Cost & Cooldown](#cost--cooldown)
- [Ability Tasks](#ability-tasks)
- [Example Walkthrough](#example-walkthrough)
- [Debugging & Testing](#debugging--testing)
- [Common Pitfalls](#common-pitfalls)
- [Frequently Asked Questions](#frequently-asked-questions)
- [Implementation Checklist](#implementation-checklist)

## What is a Gameplay Ability?

A **Gameplay Ability** is a class derived from `UGameplayAbility` that defines a specific action, skill, or behavior that a character can perform in the game world. Think of abilities as self-contained units of gameplay logic that can be activated, executed, and ended in a controlled manner.

### Core Characteristics

**Instance-Based Execution**: Unlike static functions, gameplay abilities create instances when activated. This means multiple abilities can run simultaneously, each maintaining their own state and execution context.

**Asynchronous Operations**: Abilities can perform complex, time-based operations using Ability Tasks without blocking the game thread. This allows for sophisticated gameplay patterns like channeled spells, combo attacks, or multi-stage abilities.

**Conditional Activation**: Abilities include built-in systems for checking prerequisites before activation, such as resource costs, cooldowns, tags, and custom game logic.

### Basic Structure

```cpp
// C++ Base Structure
UCLASS()
class MYGAME_API UMyGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMyGameplayAbility();

    // Core ability lifecycle methods
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        OUT FGameplayTagContainer* OptionalRelevantTags
    ) const override;

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;
};
```

**Blueprint Parallel**: In Blueprints, create a new Blueprint Class inheriting from "Gameplay Ability" and override the same functions using Blueprint nodes: `Can Activate Ability`, `Activate Ability`, and `End Ability`.

### Project Integration

In our TD RPG TopDown project, abilities integrate with:

- **UTDAbilitySystemComponent**: The project's custom ASC that manages ability granting and activation
- **ATDCharacterBase**: Base character class that provides ASC access
- **ATDPlayerState**: For player characters, the PlayerState owns the authoritative ASC

## Activation Lifecycle

Understanding the ability lifecycle is crucial for proper implementation. The lifecycle follows a predictable pattern that ensures network consistency and proper resource management.

### Phase 1: Granting

Before an ability can be used, it must be **granted** to an Ability System Component (ASC). This process creates a `FGameplayAbilitySpec` that contains:

- The ability class reference
- Current level
- Input binding information
- Activation count and instance tracking

```cpp
// C++ Granting Example
void GrantAbilityToCharacter(TSubclassOf<UGameplayAbility> AbilityClass)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
        ASC->GiveAbility(AbilitySpec);
    }
}
```

**Blueprint Equivalent**: Use the `Give Ability` node on your ASC reference.

**Replication Note**: Ability specs are automatically replicated to the owning client. This ensures that clients know which abilities their character possesses.

### Phase 2: Activation Request

When a player attempts to activate an ability (through input or other triggers):

1. **Client Side**: The input is received and processed
2. **Can Activate Check**: `CanActivateAbility` is called to verify prerequisites
3. **Activation Call**: If checks pass, `ActivateAbility` is called

```cpp
// C++ Activation
bool UMyGameplayAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    // Check base conditions first
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    // Custom validation logic
    if (const APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
    {
        // Example: Check if character is alive
        if (Pawn->GetCurrentHealth() <= 0)
        {
            return false;
        }
    }

    return true;
}
```

### Phase 3: Execution

During activation, abilities can:

- Apply Gameplay Effects for damage, healing, or attribute modification
- Start Ability Tasks for complex, asynchronous operations
- Grant or remove Gameplay Tags
- Trigger animations and visual effects
- Interact with other game systems

### Phase 4: Termination

Abilities can end through several mechanisms:

- **Natural Completion**: The ability finishes its intended function
- **Manual Ending**: `EndAbility()` is called explicitly
- **Cancellation**: External systems or conditions force termination
- **Resource Exhaustion**: Required resources become unavailable

```cpp
void UMyGameplayAbility::ActivateAbility(/*...*/)
{
    // Commit the ability (apply costs, put on cooldown)
    CommitAbility(Handle, ActorInfo, ActivationInfo);

    // Example: Start a montage and end ability when it completes
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, "PlayAttackMontage", AttackMontage);

    MontageTask->OnCompleted.AddDynamic(this, &UMyGameplayAbility::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UMyGameplayAbility::OnMontageCancelled);
    MontageTask->ReadyForActivation();
}

UFUNCTION()
void UMyGameplayAbility::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

## Replication & Prediction

GAS uses a sophisticated replication system that balances network efficiency with responsive gameplay. Understanding this system is essential for multiplayer implementations.

### Server Authority Model

**Fundamental Principle**: The server is the authoritative source for all gameplay ability state. Clients may predict certain actions for responsiveness, but the server's decision is final.

#### Granting Authorities

- **Server Only**: All ability granting must happen on the server
- **Automatic Replication**: Granted ability specs replicate to the owning client
- **Security**: Clients cannot grant abilities to themselves or others

```cpp
// ✅ CORRECT: Server-authoritative granting
void ATDPlayerState::GrantStartupAbilities()
{
    // This should only be called on the server
    if (!HasAuthority()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        for (const TSubclassOf<UGameplayAbility>& Ability : StartupAbilities)
        {
            FGameplayAbilitySpec AbilitySpec(Ability, 1, INDEX_NONE, this);
            ASC->GiveAbility(AbilitySpec);
        }
    }
}
```

#### Activation Authorities

The authority for activation depends on the ability's **Net Execution Policy**:

- **Local Predicted**: Client predicts, server validates
- **Local Only**: Runs only on the originating machine (rare)
- **Server Initiated**: Server starts, replicates to clients
- **Server Only**: Only executes on server

### Prediction System

**Client Prediction** allows clients to immediately respond to input while waiting for server confirmation. This creates responsive gameplay in networked environments.

**Prediction Keys**: GAS uses prediction keys to synchronize predicted actions between client and server. These keys ensure that predicted effects are properly handled when server confirmation arrives.

```cpp
// Prediction-friendly ability structure
void UMyPredictedAbility::ActivateAbility(/*...*/)
{
    // Predicted actions that improve responsiveness
    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        // This runs on both client (predicted) and server (authoritative)
        PlayAnimationMontage();
        ApplyImmediateGameplayEffect();
    }

    // Always commit on server for authoritative validation
    if (HasAuthority(ActorInfo))
    {
        CommitAbility(Handle, ActorInfo, ActivationInfo);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
```

### Common Authority Mistakes

```cpp
// ❌ WRONG: Client trying to grant abilities
void UMyAbility::ActivateAbility(/*...*/)
{
    GetAbilitySystemComponent()->GiveAbility(SomeOtherAbility); // Client exploit risk!
}

// ❌ WRONG: Not checking authority for important effects
void UMyDamageAbility::ActivateAbility(/*...*/)
{
    ApplyDamageToTarget(); // This might run on client and cause desync
}

// ✅ CORRECT: Proper authority handling
void UMyDamageAbility::ActivateAbility(/*...*/)
{
    if (HasAuthority(ActorInfo))
    {
        ApplyDamageToTarget(); // Only server applies damage
    }
}
```

## Cost & Cooldown

Gameplay abilities support sophisticated resource management through the cost and cooldown system. These systems prevent ability spam and create meaningful resource decisions.

### Cost System

**Costs** represent resources consumed when an ability activates (mana, stamina, special currencies, etc.). Costs are implemented as Gameplay Effects applied during ability commitment.

#### Implementing Costs

**C++ Approach**:
```cpp
UCLASS()
class UMyMagicAbility : public UGameplayAbility
{
public:
    // Cost effect to apply (typically reduces mana)
    UPROPERTY(EditDefaultsOnly, Category = "Costs")
    TSubclassOf<UGameplayEffect> CostGameplayEffectClass;

    // Override to define cost checking
    virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,
                          const FGameplayAbilityActorInfo* ActorInfo,
                          OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override
    {
        // Check base costs first
        if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
        {
            return false;
        }

        // Custom cost validation (e.g., special resources)
        return HasEnoughSpecialResource();
    }

    virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle,
                          const FGameplayAbilityActorInfo* ActorInfo,
                          const FGameplayAbilityActivationInfo ActivationInfo) const override
    {
        // Apply the base cost effect
        Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

        // Apply custom costs
        ConsumeSpecialResource();
    }
};
```

**Blueprint Setup**: In Blueprint abilities, set the `Cost Gameplay Effect Class` property to a Gameplay Effect that reduces the appropriate attributes (Mana, Stamina, etc.).

### Cooldown System

**Cooldowns** prevent abilities from being used too frequently by applying temporary blocking tags.

#### Cooldown Implementation

```cpp
// C++ Cooldown Setup
UCLASS()
class UMyAbilityWithCooldown : public UGameplayAbility
{
public:
    // Cooldown effect class
    UPROPERTY(EditDefaultsOnly, Category = "Cooldowns")
    TSubclassOf<UGameplayEffect> CooldownGameplayEffectClass;

    // Override cooldown checking
    virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle,
                              const FGameplayAbilityActorInfo* ActorInfo,
                              OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override
    {
        // This automatically checks for cooldown tags
        return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
    }

    virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                              const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo) const override
    {
        // Apply the cooldown effect
        Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
    }

    // Get remaining cooldown time
    UFUNCTION(BlueprintCallable, Category = "Cooldown")
    float GetCooldownTimeRemaining() const
    {
        if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            FGameplayTagContainer CooldownTags;
            if (UGameplayEffect* CooldownGE = CooldownGameplayEffectClass.GetDefaultObject())
            {
                CooldownGE->GetOwnedGameplayTags().GetGameplayTagArray(CooldownTags);
                
                return ASC->GetTagTimeRemaining(CooldownTags.First());
            }
        }
        return 0.0f;
    }
};
```

### Advanced Cost/Cooldown Patterns

**Dynamic Costs**: Costs can scale based on ability level, character stats, or game state:

```cpp
virtual bool CheckCost(/*...*/) const override
{
    if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
        return false;

    // Scale cost based on ability level
    float BaseCost = 50.0f;
    float ScaledCost = BaseCost * GetAbilityLevel();
    
    const UAttributeSet* AttributeSet = ActorInfo->AttributeSet.Get();
    return AttributeSet->GetMana() >= ScaledCost;
}
```

**Conditional Cooldowns**: Cooldowns can vary based on success, critical hits, or other conditions:

```cpp
void UConditionalCooldownAbility::EndAbility(/*...*/)
{
    // Reduce cooldown if ability was successful
    if (bAbilitySuccessful)
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            // Apply a "cooldown reduction" effect
            ASC->ApplyGameplayEffectToSelf(CooldownReductionEffect, 1.0f, ASC->MakeEffectContext());
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

## Ability Tasks

Ability Tasks are the backbone of complex gameplay ability behavior. They enable asynchronous operations, event-driven execution, and sophisticated ability patterns that would be difficult to implement with synchronous code.

### Understanding Ability Tasks

**Ability Tasks** (`UAbilityTask`) are objects that handle time-based or event-driven operations within abilities. They use delegate-based callbacks to notify abilities when specific conditions are met or events occur.

#### Key Characteristics

- **Asynchronous**: Tasks don't block the game thread
- **Event-Driven**: Use delegates to signal completion, cancellation, or state changes
- **Automatic Cleanup**: Tasks are automatically destroyed when abilities end
- **Blueprint Integration**: Tasks appear as nodes with multiple execution pins in Blueprint graphs

### Common Ability Task Patterns

#### PlayMontageAndWait Task

This is one of the most commonly used tasks, allowing abilities to play animation montages and respond to completion events:

```cpp
// C++ Usage
void UMyAttackAbility::ActivateAbility(/*...*/)
{
    // Create and configure the task
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,                    // Owning ability
            TEXT("AttackMontage"),   // Task name
            AttackMontage,           // Montage to play
            1.0f,                    // Play rate
            NAME_None,               // Start section
            true,                    // Stop when ability ends
            1.0f,                    // Animation root motion scale
            0.0f                     // Start time offset
        );

    // Bind delegates for different completion scenarios
    MontageTask->OnCompleted.AddDynamic(this, &UMyAttackAbility::OnAttackMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UMyAttackAbility::OnAttackMontageCancelled);
    MontageTask->OnInterrupted.AddDynamic(this, &UMyAttackAbility::OnAttackMontageInterrupted);

    // Start the task
    MontageTask->ReadyForActivation();
}

UFUNCTION()
void UMyAttackAbility::OnAttackMontageCompleted()
{
    // Montage finished normally - end the ability
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UFUNCTION()
void UMyAttackAbility::OnAttackMontageCancelled()
{
    // Montage was cancelled - end ability as cancelled
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
```

**Blueprint Equivalent**: The `Play Montage and Wait` node automatically provides multiple execution pins (`On Completed`, `On Cancelled`, `On Interrupted`) that connect to different parts of your ability graph.

#### WaitGameplayEvent Task

This task waits for specific gameplay events, enabling event-driven ability behavior:

```cpp
void UMyChannelAbility::ActivateAbility(/*...*/)
{
    // Wait for a gameplay event to interrupt channeling
    UAbilityTask_WaitGameplayEvent* EventTask = 
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            FGameplayTag::RequestGameplayTag("Ability.Interrupt"),
            nullptr,    // Any actor can send the event
            true,       // Only trigger once
            true        // Only match exact tag
        );

    EventTask->EventReceived.AddDynamic(this, &UMyChannelAbility::OnInterruptReceived);
    EventTask->ReadyForActivation();

    // Start channeling effect
    StartChannelingEffect();
}

UFUNCTION()
void UMyChannelAbility::OnInterruptReceived(FGameplayEventData Payload)
{
    // Channeling was interrupted
    UE_LOG(LogTemp, Log, TEXT("Channeling interrupted!"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
```

#### Delay Task Pattern

Simple time-based delays using the generic delay task:

```cpp
void UMyTimedAbility::ActivateAbility(/*...*/)
{
    // Wait 3 seconds then apply effect
    UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 3.0f);
    DelayTask->OnFinish.AddDynamic(this, &UMyTimedAbility::OnDelayFinished);
    DelayTask->ReadyForActivation();
}

UFUNCTION()
void UMyTimedAbility::OnDelayFinished()
{
    // Apply the delayed effect
    ApplyEffectToTarget();
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

### Custom Ability Tasks

Creating custom ability tasks allows you to encapsulate complex, reusable behavior patterns:

```cpp
// Example: Custom task that waits for attribute changes
UCLASS()
class UAbilityTask_WaitAttributeChange : public UAbilityTask
{
    GENERATED_BODY()

public:
    // Delegate declaration for Blueprint binding
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitAttributeChangeDelegate, 
        float, NewValue, float, OldValue);

    UPROPERTY(BlueprintAssignable)
    FWaitAttributeChangeDelegate OnAttributeChange;

    // Factory function
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks")
    static UAbilityTask_WaitAttributeChange* WaitForAttributeChange(
        UGameplayAbility* OwningAbility,
        FGameplayAttribute Attribute,
        float TriggerThreshold = 0.0f);

    virtual void Activate() override;

protected:
    FGameplayAttribute AttributeToWatch;
    float Threshold;
    FDelegateHandle AttributeChangedHandle;

    UFUNCTION()
    void OnAttributeChanged(const FOnAttributeChangeData& ChangeData);
};
```

### Task Management Best Practices

#### Proper Task Cleanup
```cpp
// Tasks are automatically cleaned up when abilities end
void UMyAbility::EndAbility(/*...*/)
{
    // All running tasks will be automatically ended and cleaned up
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

#### Task Chaining
```cpp
void UMyComboAbility::StartFirstAttack()
{
    UAbilityTask_PlayMontageAndWait* FirstAttack = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, TEXT("FirstAttack"), FirstAttackMontage);
    
    FirstAttack->OnCompleted.AddDynamic(this, &UMyComboAbility::StartSecondAttack);
    FirstAttack->ReadyForActivation();
}

void UMyComboAbility::StartSecondAttack()
{
    UAbilityTask_PlayMontageAndWait* SecondAttack = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, TEXT("SecondAttack"), SecondAttackMontage);
    
    SecondAttack->OnCompleted.AddDynamic(this, &UMyComboAbility::OnComboComplete);
    SecondAttack->ReadyForActivation();
}
```

## Example Walkthrough

Let's walk through creating both Blueprint and C++ versions of a simple ability to demonstrate the complete workflow from conception to implementation.

### Blueprint Example: Print and Delay Ability

This simple ability demonstrates the fundamental lifecycle pattern:

1. **Setup**: Create a new Blueprint Class inheriting from "Gameplay Ability"
2. **Implementation**: Override the `Activate Ability` function
3. **Logic**: Print a message, wait 5 seconds, then end

**Blueprint Node Flow**:
```
Activate Ability → Print String ("Ability Activated!") → Wait Delay (5.0) → End Ability
```

**What Happens**:
- **Server**: Receives activation request, validates, executes the ability
- **Client (Owning)**: May predict activation for immediate feedback
- **Client (Others)**: Receives replicated state changes if relevant

### C++ Example: Minimal Gameplay Ability

Here's the equivalent implementation in C++:

```cpp
// Header: MyPrintAbility.h
UCLASS()
class MYGAME_API UMyPrintAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMyPrintAbility();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    // Delay duration before ending ability
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
    float DelayDuration = 5.0f;

    // Message to print
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
    FString AbilityMessage = TEXT("C++ Ability Activated!");

    // Callback for delay completion
    UFUNCTION()
    void OnDelayFinished();
};

// Implementation: MyPrintAbility.cpp
UMyPrintAbility::UMyPrintAbility()
{
    // Set ability properties
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    
    // Set ability tags
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Test.Print"));
}

void UMyPrintAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // Always call super first
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Print the message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, DelayDuration, FColor::Green, AbilityMessage);
    }

    // Log activation
    UE_LOG(LogTemp, Log, TEXT("MyPrintAbility activated by %s"), 
           ActorInfo->OwnerActor.IsValid() ? *ActorInfo->OwnerActor->GetName() : TEXT("Unknown"));

    // Start delay task
    UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DelayDuration);
    DelayTask->OnFinish.AddDynamic(this, &UMyPrintAbility::OnDelayFinished);
    DelayTask->ReadyForActivation();
}

void UMyPrintAbility::OnDelayFinished()
{
    // Log completion
    UE_LOG(LogTemp, Log, TEXT("MyPrintAbility delay finished, ending ability"));

    // End the ability cleanly
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, 
               true,  // Replicate end
               false  // Not cancelled
    );
}
```

### Network Behavior Analysis

**On Server**:
1. Server receives activation request
2. `CanActivateAbility` checks pass
3. `ActivateAbility` executes, printing message
4. Delay task starts
5. After 5 seconds, `OnDelayFinished` is called
6. `EndAbility` is called, replicating the end state

**On Owning Client (if predicted)**:
1. Client immediately starts prediction
2. Local message is printed for responsiveness  
3. Delay starts locally
4. Server confirmation arrives
5. Any discrepancies are corrected by server authority

**On Other Clients**:
- Only see replicated state changes that affect them
- May not see the ability execution at all unless it has visible effects

### Integration with TD RPG Project

To use this ability in our project:

```cpp
// In ATDPlayerState or ATDCharacterBase
void GrantTestAbility()
{
    if (HasAuthority() && IsValid(AbilitySystemComponent))
    {
        FGameplayAbilitySpec Spec(UMyPrintAbility::StaticClass(), 1, INDEX_NONE, this);
        AbilitySystemComponent->GiveAbility(Spec);
    }
}

// Activate via input binding or other trigger
void TriggerTestAbility()
{
    if (IsValid(AbilitySystemComponent))
    {
        AbilitySystemComponent->TryActivateAbilitiesByTag(
            FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Test.Print"))
        );
    }
}
```

## Debugging & Testing

Effective debugging is crucial for ability development. GAS provides several built-in tools and techniques for troubleshooting ability-related issues.

### Logging and Debug Information

#### Built-in Console Commands

**`showdebug abilitysystem`**: Displays comprehensive ASC information including:
- Active abilities and their states
- Applied gameplay effects
- Current attribute values
- Active gameplay tags

**`AbilitySystem.DebugAbilityTags 1`**: Shows ability-related tags on screen

**`AbilitySystem.PrintDebug`**: Outputs detailed ASC state to the log

#### Custom Logging

```cpp
// Use structured logging for ability events
DEFINE_LOG_CATEGORY_STATIC(LogMyAbilities, Log, All);

void UMyAbility::ActivateAbility(/*...*/)
{
    UE_LOG(LogMyAbilities, Log, 
           TEXT("Ability %s activated by %s at level %d"), 
           *GetClass()->GetName(),
           *GetActorInfo()->OwnerActor->GetName(),
           GetAbilityLevel());

    // Log network role for multiplayer debugging
    UE_LOG(LogMyAbilities, VeryVerbose, 
           TEXT("Activation - NetRole: %s, HasAuthority: %s"),
           *UEnum::GetValueAsString(GetActorInfo()->OwnerActor->GetLocalRole()),
           HasAuthority() ? TEXT("true") : TEXT("false"));
}
```

#### Visual Debug Messages

```cpp
// On-screen debug messages with categories
void DisplayAbilityDebugInfo(const FString& Message, float Duration = 5.0f)
{
    if (GEngine)
    {
        FColor DebugColor = HasAuthority() ? FColor::Green : FColor::Orange;
        GEngine->AddOnScreenDebugMessage(
            -1, Duration, DebugColor, 
            FString::Printf(TEXT("[%s] %s"), 
                           HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
                           *Message));
    }
}
```

### Common Debug Scenarios

#### Ability Not Activating

**Checklist**:
1. Is the ability properly granted? Check with `showdebug abilitysystem`
2. Does `CanActivateAbility` return true?
3. Are there conflicting tags preventing activation?
4. Are cost requirements met?
5. Is the ability on cooldown?

```cpp
// Enhanced CanActivateAbility for debugging
bool UMyAbility::CanActivateAbility(/*...*/) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        UE_LOG(LogMyAbilities, Warning, TEXT("Base CanActivateAbility failed"));
        return false;
    }

    // Check custom conditions with logging
    if (!HasEnoughMana())
    {
        UE_LOG(LogMyAbilities, Warning, TEXT("Insufficient mana for ability activation"));
        return false;
    }

    return true;
}
```

#### Network Synchronization Issues

**Symptoms**: Client and server showing different ability states
**Debugging**:

```cpp
void UMyAbility::ActivateAbility(/*...*/)
{
    // Log on both server and client for comparison
    UE_LOG(LogMyAbilities, Warning, 
           TEXT("ACTIVATE - Role: %s, Authority: %s, PredictionKey: %s"),
           *UEnum::GetValueAsString(GetWorld()->GetNetMode()),
           HasAuthority(ActorInfo) ? TEXT("TRUE") : TEXT("FALSE"),
           ActivationInfo.GetPredictionKeyForNewAction().ToString());
}
```

#### Ability Tasks Not Working

**Common Issues**:
1. Forgetting to call `ReadyForActivation()`
2. Not binding delegates before activation
3. Task lifetime issues (ability ending too early)

```cpp
// Defensive task setup
void UMyAbility::SetupMontageTask()
{
    if (!IsValid(AttackMontage))
    {
        UE_LOG(LogMyAbilities, Error, TEXT("AttackMontage is null!"));
        EndAbility(/*...*/);
        return;
    }

    UAbilityTask_PlayMontageAndWait* Task = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(/*...*/);
    
    if (!IsValid(Task))
    {
        UE_LOG(LogMyAbilities, Error, TEXT("Failed to create montage task!"));
        EndAbility(/*...*/);
        return;
    }

    // Bind delegates
    Task->OnCompleted.AddDynamic(this, &UMyAbility::OnMontageComplete);
    Task->OnCancelled.AddDynamic(this, &UMyAbility::OnMontageCancelled);
    
    // Activate task
    Task->ReadyForActivation();
    
    UE_LOG(LogMyAbilities, Log, TEXT("Montage task created and activated"));
}
```

### Testing Strategies

#### Unit Testing Abilities

```cpp
// Example test setup for ability validation
class FMyAbilityTest
{
public:
    static bool TestAbilityActivation()
    {
        // Create test ASC and character
        UAbilitySystemComponent* TestASC = NewObject<UAbilitySystemComponent>();
        APawn* TestPawn = GetWorld()->SpawnActor<APawn>();
        
        // Grant test ability
        FGameplayAbilitySpec Spec(UMyTestAbility::StaticClass(), 1);
        TestASC->GiveAbility(Spec);
        
        // Attempt activation
        bool bActivated = TestASC->TryActivateAbility(Spec.Handle);
        
        // Verify results
        return bActivated && TestASC->HasMatchingGameplayTag(
            FGameplayTag::RequestGameplayTag("State.Ability.Active"));
    }
};
```

#### Multiplayer Testing

**Local Testing**: Use the Unreal Editor's multiplayer PIE (Play In Editor) with multiple windows to simulate client-server scenarios.

**Network Simulation**: Enable network emulation to test with realistic latency and packet loss:
```cpp
// Console commands for network simulation
// "Net.PktLag 100" - Add 100ms lag
// "Net.PktLoss 5" - 5% packet loss
// "Net.PktDup 2" - 2% packet duplication
```

### Performance Monitoring

Monitor ability performance to ensure smooth gameplay:

```cpp
void UMyAbility::ActivateAbility(/*...*/)
{
    SCOPE_CYCLE_COUNTER(STAT_MyAbilityActivation);
    
    // Ability logic here
    
    // Log performance metrics
    UE_LOG(LogMyAbilities, VeryVerbose, 
           TEXT("Ability activation took %f ms"), 
           FPlatformTime::Seconds() * 1000.0);
}
```

## Common Pitfalls

Understanding and avoiding these common mistakes will save significant development time and prevent subtle bugs in your ability system.

### Pitfall 1: Forgetting to End Abilities

**Problem**: Abilities that don't properly end continue consuming resources and may block future ability activations.

```cpp
// ❌ WRONG: Ability never ends
void UBadAbility::ActivateAbility(/*...*/)
{
    // Do some work
    ApplyDamageToTarget();
    
    // Oops! Forgot to call EndAbility()
    // This ability will remain active forever
}

// ✅ CORRECT: Always ensure abilities end
void UGoodAbility::ActivateAbility(/*...*/)
{
    ApplyDamageToTarget();
    
    // Always end the ability when work is complete
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

**Solution**: Always have a clear path to `EndAbility()` for every activation scenario.

### Pitfall 2: Dangling Ability Tasks

**Problem**: Ability tasks that outlive their parent abilities can cause crashes or memory leaks.

```cpp
// ❌ WRONG: Task might outlive ability
void UProblematicAbility::ActivateAbility(/*...*/)
{
    UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, 10.0f);
    Task->OnFinish.AddDynamic(this, &UProblematicAbility::OnDelayFinished);
    Task->ReadyForActivation();
    
    // Ability ends immediately, but task is still running!
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

// ✅ CORRECT: Task-driven ability lifecycle
void UCorrectAbility::ActivateAbility(/*...*/)
{
    UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, 10.0f);
    Task->OnFinish.AddDynamic(this, &UCorrectAbility::OnDelayFinished);
    Task->ReadyForActivation();
    
    // Don't end immediately - let task drive the lifecycle
}

void UCorrectAbility::OnDelayFinished()
{
    // Now it's safe to end
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

### Pitfall 3: Authority Confusion

**Problem**: Calling authority-sensitive functions on the wrong network role.

```cpp
// ❌ WRONG: Client trying to grant abilities
void UInsecureAbility::ActivateAbility(/*...*/)
{
    // This could run on client - security risk!
    GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(SomeAbilityClass, 1));
    
    // This might not replicate properly
    ApplyGameplayEffectToSelf(DamageEffect);
}

// ✅ CORRECT: Proper authority checks
void USecureAbility::ActivateAbility(/*...*/)
{
    // Only server can grant abilities
    if (HasAuthority(CurrentActorInfo))
    {
        GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(SomeAbilityClass, 1));
        ApplyGameplayEffectToSelf(DamageEffect);
    }
}
```

### Pitfall 4: Missing Cost/Cooldown Effects

**Problem**: Abilities that don't properly consume resources or apply cooldowns can be exploited.

```cpp
// ❌ WRONG: Manual resource deduction without commit
void UUncommittedAbility::ActivateAbility(/*...*/)
{
    // Manually deducting resources is error-prone
    if (UMyAttributeSet* Attrs = Cast<UMyAttributeSet>(GetAttributeSet()))
    {
        Attrs->SetMana(Attrs->GetMana() - ManaCost);
    }
    
    // No cooldown applied - can spam this ability!
}

// ✅ CORRECT: Use CommitAbility for proper resource handling
void UCommittedAbility::ActivateAbility(/*...*/)
{
    // CommitAbility handles costs and cooldowns automatically
    if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        // Failed to commit (insufficient resources, etc.)
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    // Proceed with ability logic
    DoAbilityWork();
}
```

### Pitfall 5: Blueprint Node Execution Order

**Problem**: In Blueprints, connecting nodes in the wrong order can cause unexpected behavior.

```
❌ WRONG Blueprint Flow:
Activate Ability → [Split] → Play Montage and Wait
                          → End Ability

Problem: End Ability executes immediately, cancelling the montage
```

```
✅ CORRECT Blueprint Flow:
Activate Ability → Play Montage and Wait → [On Completed] → End Ability
                                       → [On Cancelled] → End Ability
```

### Pitfall 6: Attribute Modification During Prediction

**Problem**: Modifying attributes directly during client prediction can cause synchronization issues.

```cpp
// ❌ WRONG: Direct attribute modification during prediction
void UPredictedAbility::ActivateAbility(/*...*/)
{
    if (UMyAttributeSet* Attrs = Cast<UMyAttributeSet>(GetAttributeSet()))
    {
        // This will cause client-server desync!
        Attrs->SetHealth(Attrs->GetHealth() - 25.0f);
    }
}

// ✅ CORRECT: Use Gameplay Effects for attribute changes
void UPredictedAbility::ActivateAbility(/*...*/)
{
    // Gameplay Effects handle prediction properly
    FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
        HealthReductionEffect, GetAbilityLevel(), EffectContext);
    
    GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
```

### Pitfall 7: Inefficient Ability Checking

**Problem**: Expensive operations in `CanActivateAbility` can hurt performance.

```cpp
// ❌ WRONG: Expensive operations in CanActivateAbility
bool USlowAbility::CanActivateAbility(/*...*/) const
{
    if (!Super::CanActivateAbility(/*...*/))
        return false;
    
    // Expensive line trace every frame when checking if ability can activate
    FHitResult HitResult;
    GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
    
    return HitResult.bBlockingHit;
}

// ✅ CORRECT: Cache expensive calculations or move to activation
bool UOptimizedAbility::CanActivateAbility(/*...*/) const
{
    // Only do cheap checks here
    return Super::CanActivateAbility(/*...*/) && HasBasicRequirements();
}

void UOptimizedAbility::ActivateAbility(/*...*/)
{
    // Do expensive validation only when actually activating
    if (!PerformExpensiveChecks())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    // Continue with ability
}
```

## Frequently Asked Questions

### Q: Can I activate abilities directly on the client?

**A**: It depends on the ability's `NetExecutionPolicy`:

- **LocalPredicted**: Client can predict activation, but server validates
- **ServerOnly**: Only server can activate
- **LocalOnly**: Only runs on the originating machine (rare use case)
- **ServerInitiated**: Server starts, replicates to clients

For most gameplay abilities, use `LocalPredicted` for responsive multiplayer gameplay.

### Q: Do I need prediction keys for my abilities?

**A**: Prediction keys are handled automatically by GAS. You typically don't need to manage them manually unless you're creating custom ability tasks or doing advanced networking customizations.

```cpp
// Prediction keys are handled automatically
void UMyAbility::ActivateAbility(/*...*/)
{
    // GAS automatically assigns and manages prediction keys
    // Use HasAuthorityOrPredictionKey() to check if code should run
    if (HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
    {
        // This runs on both predicting client and authoritative server
        DoImmediateWork();
    }
}
```

### Q: Can multiple abilities run simultaneously?

**A**: Yes! This is one of GAS's key features. The `InstancingPolicy` determines how:

- **InstancedPerActor**: New instance for each activation (allows multiple simultaneously)
- **InstancedPerExecution**: New instance only if not currently active
- **NonInstanced**: Reuses the same CDO instance (generally not recommended)

```cpp
// Multiple abilities can run simultaneously
GetAbilitySystemComponent()->TryActivateAbility(Ability1Spec.Handle);
GetAbilitySystemComponent()->TryActivateAbility(Ability2Spec.Handle);
GetAbilitySystemComponent()->TryActivateAbility(Ability3Spec.Handle);
// All three can be active at once if they use InstancedPerActor
```

### Q: How do I cancel abilities?

**A**: Several approaches depending on your needs:

```cpp
// Cancel specific ability by tag
GetAbilitySystemComponent()->CancelAbilitiesByTag(
    FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Attack")));

// Cancel all abilities with specific tags
GetAbilitySystemComponent()->CancelAllAbilities();

// Cancel from within an ability
void UMyAbility::SomeConditionMet()
{
    // Cancel this ability
    CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
```

### Q: How do I check if an ability is currently active?

**A**: Use the ASC's query functions:

```cpp
// Check if any ability with specific tag is active
bool bIsAttacking = GetAbilitySystemComponent()->HasMatchingGameplayTag(
    FGameplayTag::RequestGameplayTag("State.Attacking"));

// Check specific ability by class
bool bSpecificAbilityActive = GetAbilitySystemComponent()->FindAbilitySpecFromClass(
    UMySpecificAbility::StaticClass()) != nullptr;

// Get all active abilities
TArray<FGameplayAbilitySpec*> ActiveAbilities;
GetAbilitySystemComponent()->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
    FGameplayTagContainer(), ActiveAbilities);
```

### Q: Can abilities activate other abilities?

**A**: Yes, but be careful about infinite loops and authority:

```cpp
void UComboAbility::ActivateAbility(/*...*/)
{
    // Activate another ability as part of this one
    FGameplayTagContainer ComboTag(FGameplayTag::RequestGameplayTag("Ability.Combo.Second"));
    
    if (HasAuthority(CurrentActorInfo))
    {
        GetAbilitySystemComponent()->TryActivateAbilitiesByTag(ComboTag);
    }
}
```

### Q: How do I make abilities that require targeting?

**A**: Use targeting tasks or implement custom targeting logic:

```cpp
void UTargetedAbility::ActivateAbility(/*...*/)
{
    // Wait for targeting input
    UAbilityTask_WaitTargetData* TargetingTask = UAbilityTask_WaitTargetData::WaitTargetData(
        this,
        "WaitingForTarget", 
        EGameplayTargetingConfirmation::UserConfirmed,
        AMyTargetActor::StaticClass());

    TargetingTask->ValidData.AddDynamic(this, &UTargetedAbility::OnTargetDataReady);
    TargetingTask->Cancelled.AddDynamic(this, &UTargetedAbility::OnTargetingCancelled);
    TargetingTask->ReadyForActivation();
}

UFUNCTION()
void UTargetedAbility::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData)
{
    // Apply effect to targeted location/actor
    ApplyEffectToTarget(TargetData);
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

### Q: How do I handle ability interruption?

**A**: Implement cancellation handlers and use appropriate tags:

```cpp
void UInterruptibleAbility::ActivateAbility(/*...*/)
{
    // Add tags that can be used to identify and cancel this ability
    GetAbilitySystemComponent()->AddLooseGameplayTag(
        FGameplayTag::RequestGameplayTag("State.Channeling"));

    // Set up interrupt conditions
    UAbilityTask_WaitGameplayTagAdded* InterruptTask = 
        UAbilityTask_WaitGameplayTagAdded::WaitGameplayTagAdd(
            this, FGameplayTag::RequestGameplayTag("Effect.Interrupt"));

    InterruptTask->Added.AddDynamic(this, &UInterruptibleAbility::OnInterrupted);
    InterruptTask->ReadyForActivation();
}

UFUNCTION()
void UInterruptibleAbility::OnInterrupted()
{
    UE_LOG(LogTemp, Log, TEXT("Ability was interrupted!"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

virtual void EndAbility(/*...*/) override
{
    // Clean up tags when ending
    GetAbilitySystemComponent()->RemoveLooseGameplayTag(
        FGameplayTag::RequestGameplayTag("State.Channeling"));
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

## Implementation Checklist

Use this comprehensive checklist to ensure your gameplay abilities are properly implemented:

### Design Phase
- [ ] **Ability Purpose Defined**: Clear understanding of what the ability should do
- [ ] **Network Policy Chosen**: LocalPredicted, ServerOnly, etc.
- [ ] **Instancing Policy Decided**: InstancedPerActor, InstancedPerExecution, etc.
- [ ] **Cost/Cooldown Requirements**: Defined resource costs and cooldown duration
- [ ] **Animation/Visual Requirements**: Montages, particles, sounds identified

### Implementation Phase

#### Basic Setup
- [ ] **Class Created**: Blueprint or C++ class inheriting from UGameplayAbility
- [ ] **Basic Properties Set**: InstancingPolicy, NetExecutionPolicy configured
- [ ] **Ability Tags Assigned**: Proper tags for identification and organization
- [ ] **Constructor Logic**: Default values and initialization complete

#### Core Functionality
- [ ] **CanActivateAbility Override**: Custom validation logic implemented
- [ ] **ActivateAbility Override**: Main ability logic implemented
- [ ] **EndAbility Override**: Cleanup logic if needed
- [ ] **CommitAbility Called**: Proper resource consumption and cooldown application

#### Cost & Cooldown System
- [ ] **Cost Effect Created**: GameplayEffect for resource consumption
- [ ] **Cost Effect Assigned**: CostGameplayEffectClass property set
- [ ] **Cooldown Effect Created**: GameplayEffect for cooldown application  
- [ ] **Cooldown Effect Assigned**: CooldownGameplayEffectClass property set
- [ ] **Cost Validation**: CheckCost override if custom logic needed

#### Ability Tasks (if used)
- [ ] **Tasks Created**: Proper task instantiation with factory methods
- [ ] **Delegates Bound**: All necessary callbacks bound before activation
- [ ] **ReadyForActivation Called**: Tasks properly started
- [ ] **Cleanup Handled**: EndAbility properly cleans up running tasks

#### Network Considerations
- [ ] **Authority Checks**: Proper HasAuthority() checks for sensitive operations
- [ ] **Prediction Friendly**: Uses HasAuthorityOrPredictionKey() where appropriate
- [ ] **Replication Tags**: Ability tags properly configured for replication needs

### Testing Phase

#### Single Player Testing
- [ ] **Basic Activation**: Ability activates when triggered
- [ ] **Resource Consumption**: Costs are properly applied
- [ ] **Cooldown Application**: Cannot reactivate during cooldown
- [ ] **Animation Integration**: Montages play correctly if used
- [ ] **Task Execution**: All ability tasks complete properly

#### Multiplayer Testing
- [ ] **Server Authority**: Server properly validates and executes ability
- [ ] **Client Prediction**: Owning client gets responsive feedback
- [ ] **Network Replication**: State changes replicate to relevant clients
- [ ] **Lag Compensation**: Ability works properly with network latency
- [ ] **Edge Cases**: Handle disconnection, possession changes, etc.

#### Integration Testing
- [ ] **ASC Integration**: Works properly with project's AbilitySystemComponent
- [ ] **Character Integration**: Integrates with ATDCharacterBase properly
- [ ] **UI Integration**: Ability state reflects in UI systems
- [ ] **Input Integration**: Proper input binding and activation
- [ ] **Save/Load**: Ability state persists if needed

### Debug & Polish Phase
- [ ] **Debug Logging**: Proper logging for troubleshooting
- [ ] **Error Handling**: Graceful failure modes implemented
- [ ] **Performance Testing**: No significant performance bottlenecks
- [ ] **Visual Polish**: Effects, animations, sounds properly integrated
- [ ] **Documentation**: Code comments and usage documentation complete

### Project-Specific Integration (TD RPG TopDown)
- [ ] **UTDAbilitySystemComponent**: Proper integration with project ASC
- [ ] **ATDCharacterBase**: Works with base character class
- [ ] **ATDPlayerState**: Proper interaction with player state for players
- [ ] **Project Tags**: Uses project-specific gameplay tags
- [ ] **Attribute Integration**: Works with project's attribute set

### Final Validation
- [ ] **Peer Review**: Code reviewed by team member
- [ ] **QA Testing**: Tested by QA team or designated tester  
- [ ] **Performance Profile**: No significant performance regressions
- [ ] **Memory Leaks**: No memory leaks detected
- [ ] **Documentation Updated**: Project documentation reflects new ability

This checklist ensures comprehensive coverage of all aspects necessary for a production-ready gameplay ability in the GAS framework.