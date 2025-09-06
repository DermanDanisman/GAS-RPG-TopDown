# Gameplay Ability Tasks

Last updated: 2025-01-21

## Overview

Gameplay Ability Tasks are asynchronous building blocks that enable abilities to perform complex operations without blocking execution. They provide a bridge between the synchronous ability activation system and asynchronous operations like line traces, delays, animation montages, and event listening.

In the GASCore plugin, ability tasks extend the base `UAbilityTask` class to provide specialized functionality for common gameplay patterns.

## Core Concepts

### What are Ability Tasks?

- **Asynchronous Execution**: Tasks run over multiple frames without blocking the main thread
- **Delegate-Based Communication**: Tasks communicate completion/progress through multicast delegates
- **Blueprint Integration**: Tasks appear as special nodes with multiple output pins in Blueprint graphs
- **Automatic Cleanup**: Tasks are automatically cleaned up when the owning ability ends

### Task Lifecycle

```cpp
// 1. Creation (usually static factory method)
UMyAbilityTask* Task = UMyAbilityTask::CreateTask(OwningAbility, Parameters);

// 2. Delegate Binding
Task->OnCompleted.AddDynamic(this, &UMyAbility::OnTaskCompleted);
Task->OnFailed.AddDynamic(this, &UMyAbility::OnTaskFailed);

// 3. Activation
Task->ReadyForActivation();

// 4. Execution (internal - runs over multiple frames)
// Task->Activate() called internally
// Task->TickTask() called each frame if needed

// 5. Completion
// Task fires delegates and automatically cleans up
```

## GASCore Target Data From Aim Trace Task

### Implementation

The `UGASCoreTargetDataFromAimTrace` class provides target selection through aim tracing functionality:

```cpp
UCLASS()
class GASCORE_API UGASCoreTargetDataFromAimTrace : public UAbilityTask
{
    GENERATED_BODY()

public:
    // Factory method for Blueprint/C++ creation
    UFUNCTION(BlueprintCallable, Category = "GASCore|Ability Task|Target Data From Aim Trace", 
        meta = (DisplayName = "TargetDataFromAimTrace", HidePin = "OwningAbility", 
        DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
    static UGASCoreTargetDataFromAimTrace* CreateTargetDataFromAimTrace(UGameplayAbility* OwningAbility);

    // Delegate fired when valid hit result is obtained
    UPROPERTY(BlueprintAssignable)
    FAimTraceTargetDataSignature ValidHitResultData;

private:
    virtual void Activate() override;
};
```

### Delegate Declaration

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimTraceTargetDataSignature, const FHitResult&, HitResultData);
```

This delegate provides hit result data including:
- **Location**: World position of the hit
- **Normal**: Surface normal at hit point
- **Actor**: The actor that was hit (if any)
- **Component**: The specific component that was hit
- **Distance**: Distance from trace start to hit point

### Usage in Abilities

#### C++ Implementation

```cpp
void UMyProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // Create aim trace task
    UGASCoreTargetDataFromAimTrace* AimTraceTask = 
        UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(this);
    
    // Bind to completion delegate
    AimTraceTask->ValidHitResultData.AddDynamic(this, &UMyProjectileAbility::OnAimTraceCompleted);
    
    // Activate the task
    AimTraceTask->ReadyForActivation();
}

void UMyProjectileAbility::OnAimTraceCompleted(const FHitResult& HitResult)
{
    if (HitResult.bBlockingHit)
    {
        // Use hit result for projectile targeting
        FVector TargetLocation = HitResult.Location;
        SpawnProjectileToTarget(TargetLocation);
    }
    
    // End the ability
    K2_EndAbility();
}
```

#### Blueprint Usage

In Blueprint graphs, the task appears as a special node:

```
[Event ActivateAbility]
        ↓
[TargetDataFromAimTrace]
    ├─Exec Out─→ [Continue ability logic]
    └─ValidHitResultData─→ [Process hit result] → [Spawn Projectile] → [End Ability]
```

## Common Ability Task Patterns

### 1. Wait for Input Task

```cpp
// Waits for specific input while ability is active
UCLASS()
class UAbilityTask_WaitInputPress : public UAbilityTask
{
    UPROPERTY(BlueprintAssignable)
    FGenericDelegate OnPress;
    
    UPROPERTY(BlueprintAssignable)
    FGenericDelegate OnRelease;
};
```

### 2. Play Montage and Wait Task

```cpp
// Plays animation montage and waits for completion
UCLASS()
class UAbilityTask_PlayMontageAndWait : public UAbilityTask
{
    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnCompleted;
    
    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnInterrupted;
    
    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnCancelled;
};
```

### 3. Wait for Gameplay Event Task

```cpp
// Listens for specific gameplay event tags
UCLASS()
class UAbilityTask_WaitGameplayEvent : public UAbilityTask
{
    UPROPERTY(BlueprintAssignable)
    FWaitGameplayEventDelegate EventReceived;
    
    FGameplayTag EventTag;
};
```

## Implementation Best Practices

### Task Creation Patterns

#### Static Factory Method
```cpp
UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", 
    meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", 
    BlueprintInternalUseOnly = "true"))
static UMyAbilityTask* CreateMyTask(UGameplayAbility* OwningAbility, 
    const FMyTaskParams& Parameters);
```

#### Factory Implementation
```cpp
UMyAbilityTask* UMyAbilityTask::CreateMyTask(UGameplayAbility* OwningAbility, 
    const FMyTaskParams& Parameters)
{
    UMyAbilityTask* MyObj = NewAbilityTask<UMyAbilityTask>(OwningAbility);
    MyObj->TaskParameters = Parameters;
    return MyObj;
}
```

### Delegate Binding Best Practices

#### Use Dynamic Delegates for Blueprint Support
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyTaskDelegate, bool, bSuccess);

UPROPERTY(BlueprintAssignable)
FMyTaskDelegate OnTaskCompleted;
```

#### Provide Multiple Output States
```cpp
UPROPERTY(BlueprintAssignable)
FMyTaskDelegate OnSuccess;

UPROPERTY(BlueprintAssignable)
FMyTaskDelegate OnFailed;

UPROPERTY(BlueprintAssignable)
FMyTaskDelegate OnCancelled;
```

### Activation and Cleanup

#### Override Activate for Custom Logic
```cpp
void UMyAbilityTask::Activate()
{
    Super::Activate();
    
    // Validate parameters
    if (!IsValidParameters())
    {
        OnFailed.Broadcast();
        EndTask();
        return;
    }
    
    // Start async operation
    StartAsyncOperation();
}
```

#### Proper Task Termination
```cpp
void UMyAbilityTask::CompleteTask(bool bSuccess)
{
    if (bSuccess)
    {
        OnSuccess.Broadcast();
    }
    else
    {
        OnFailed.Broadcast();
    }
    
    // Always call EndTask() to clean up
    EndTask();
}
```

## Debugging Ability Tasks

### Console Commands

```
showdebug abilitysystem
```
Shows active tasks along with abilities, cooldowns, and attributes.

### Logging Best Practices

```cpp
void UMyAbilityTask::Activate()
{
    UE_LOG(LogAbilitySystem, Log, TEXT("MyAbilityTask: Activating task for ability %s"), 
        *GetNameSafe(Ability));
    
    Super::Activate();
}

void UMyAbilityTask::CompleteTask(bool bSuccess)
{
    UE_LOG(LogAbilitySystem, Log, TEXT("MyAbilityTask: Completing task - Success: %s"), 
        bSuccess ? TEXT("True") : TEXT("False"));
    
    // Complete task logic...
}
```

### Blueprint Debugging

- Use **Print String** nodes connected to delegate outputs
- Add **Breakpoints** on task completion pins
- Monitor task state through the **Ability System Debug** display

## Common Pitfalls and Solutions

### Pitfall 1: Forgetting to Call EndTask()

**Problem**: Task continues running after completion, causing memory leaks and potential crashes.

**Solution**: Always call `EndTask()` in completion paths:

```cpp
void UMyAbilityTask::OnOperationComplete(bool bSuccess)
{
    // Fire appropriate delegate
    if (bSuccess)
        OnSuccess.Broadcast();
    else
        OnFailed.Broadcast();
    
    // ALWAYS call EndTask()
    EndTask();
}
```

### Pitfall 2: Task Activation Without Delegate Binding

**Problem**: Task completes but no code responds to the completion.

**Solution**: Always bind delegates before activation:

```cpp
// WRONG - delegates not bound
UMyAbilityTask* Task = UMyAbilityTask::CreateTask(this);
Task->ReadyForActivation(); // No one listening!

// CORRECT - bind first, then activate
UMyAbilityTask* Task = UMyAbilityTask::CreateTask(this);
Task->OnSuccess.AddDynamic(this, &UMyAbility::OnTaskSuccess);
Task->OnFailed.AddDynamic(this, &UMyAbility::OnTaskFailed);
Task->ReadyForActivation();
```

### Pitfall 3: Manual Task Cleanup

**Problem**: Attempting to manually destroy or clean up tasks.

**Solution**: Let the ability system handle cleanup automatically:

```cpp
// WRONG - manual cleanup
Task->ConditionalBeginDestroy();

// CORRECT - let system handle it
Task->EndTask(); // System cleans up automatically
```

### Pitfall 4: Replication Issues

**Problem**: Tasks running on client when they should be server-only.

**Solution**: Validate authority in task activation:

```cpp
void UMyAbilityTask::Activate()
{
    Super::Activate();
    
    // Check if this should run on server only
    if (ShouldBroadcastAbilityTaskDelegates() && !HasAuthority())
    {
        OnFailed.Broadcast();
        EndTask();
        return;
    }
    
    // Continue with task logic...
}
```

## Related Documentation

- [Gameplay Abilities Concepts and Practice](../guides/GameplayAbilities_Concepts_And_Practice.md)
- [Projectile Abilities Pattern](projectile-abilities.md)
- [Combat Interface](combat-interface.md)
- [Ability System API](api.md)

## See Also

- [Unreal Engine Ability Tasks Documentation](https://docs.unrealengine.com/5.3/en-US/ability-tasks-in-unreal-engine/)
- [GAS Documentation](https://github.com/tranek/GASDocumentation#ability-tasks)