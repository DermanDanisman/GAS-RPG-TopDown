# Gameplay Abilities Overview

Last updated: 2024-12-19

## Purpose and Responsibilities

Gameplay Abilities in GAS are the primary mechanism for implementing complex, stateful player actions that require validation, prediction, and replication. They handle:

- **Input-driven actions**: Spells, attacks, defensive maneuvers
- **Contextual validation**: Can this ability activate now? (cooldowns, resources, tags)
- **Cost management**: Consume mana/stamina via GameplayEffects
- **Networked execution**: Server authority with client prediction
- **Complex sequences**: Multi-stage abilities with branching logic

Unlike simple input handlers, abilities provide a structured framework for actions that need to interact with the broader GAS ecosystem (attributes, effects, tags).

## Ability Lifecycle

### 1. Granting and Specs

Abilities must be granted to an ASC before they can be activated:

```cpp
// C++ - Grant ability to ASC
FGameplayAbilitySpec AbilitySpec(AbilityClass, Level, InputID, SourceObject);
ASC->GiveAbility(AbilitySpec);

// C++ - Grant and activate immediately (useful for testing)
ASC->GiveAbilityAndActivateOnce(AbilitySpec);
```

**Blueprint equivalent:**
- `Give Ability` node in ASC
- Store the returned `FGameplayAbilitySpecHandle` for later reference

### 2. Activation

Abilities can be activated by:
- **Input binding**: Mapped to specific input actions via InputID
- **Manual triggering**: `TryActivateAbility()` calls
- **Gameplay events**: Responding to external triggers

**Activation checks:**
- Ability cooldown state
- Required/blocked gameplay tags
- Resource costs (via `CanActivateAbility()`)

### 3. Execution

During execution, abilities can:
- Apply GameplayEffects (damage, healing, buffs)
- Spawn actors or projectiles
- Play animations and audio
- Wait for external events or timers
- Chain into other abilities

### 4. Ending and Canceling

**Normal ending:**
- `EndAbility()` called when ability completes naturally
- Commits costs and cooldowns
- Cleans up ability state

**Cancellation:**
- `CancelAbility()` for interruptions
- May or may not commit costs (configurable)
- Immediately stops execution

## Replication and Prediction

### Server Authority
- **Authoritative execution**: Server validates and runs ability logic
- **State replication**: Ability activation/end states replicate to clients
- **Effect application**: Server applies all GameplayEffects authoritatively

### Client Prediction
- **Optimistic activation**: Clients can predict ability start for responsiveness
- **Rollback handling**: Server corrections reconcile mispredicted outcomes
- **Visual feedback**: Immediate response to player input while awaiting server confirmation

**Configuration in ability class:**
```cpp
// Enable prediction for responsive abilities (attacks, movement)
NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

// Server-only for abilities with complex validation
NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
```

## Cost and Cooldown Management

### Cost via GameplayEffects

Abilities consume resources through `CostGameplayEffectClass`:

```cpp
// In ability class definition
UPROPERTY(EditDefaultsOnly, Category = "Costs")
TSubclassOf<UGameplayEffect> CostGameplayEffectClass;
```

**Example cost effect:** Instant GE that reduces Mana by 25

### Cooldown via GameplayEffects

Cooldowns are managed through `CooldownGameplayEffectClass`:

```cpp
// In ability class definition  
UPROPERTY(EditDefaultsOnly, Category = "Cooldowns")
TSubclassOf<UGameplayEffect> CooldownGameplayEffectClass;
```

**Example cooldown effect:** Duration GE with gameplay tag "Cooldown.Fireball" for 3 seconds

### CommitAbility Pattern

```cpp
bool UMyGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // Validate resources and cooldowns
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return false;
    }
    
    // Ability logic here...
    return true;
}
```

`CommitAbility()` automatically applies both cost and cooldown effects if they pass their checks.

## Ability Tasks

Ability Tasks enable asynchronous operations within abilities, essential for complex sequences:

### WaitDelay
```cpp
// C++ - Wait for a specified duration
UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, 2.0f);
WaitTask->OnFinish.AddDynamic(this, &UMyAbility::OnDelayFinished);
WaitTask->ReadyForActivation();
```

### PlayMontageAndWait
```cpp
// C++ - Play animation and wait for completion/events
UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    this, "PlayAttackMontage", AttackMontage, 1.0f);
MontageTask->OnCompleted.AddDynamic(this, &UMyAbility::OnMontageCompleted);
MontageTask->OnNotifyBegin.AddDynamic(this, &UMyAbility::OnAttackNotify);
MontageTask->ReadyForActivation();
```

### WaitGameplayEvent
```cpp
// C++ - Wait for external gameplay events
UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
    this, FGameplayTag::RequestGameplayTag("Event.Combat.HitConfirm"));
EventTask->EventReceived.AddDynamic(this, &UMyAbility::OnHitConfirmed);
EventTask->ReadyForActivation();
```

**Blueprint equivalents:** All ability tasks have corresponding Blueprint nodes with similar functionality.

## C++ and Blueprint Mapping

### Base Ability Class Pattern
```cpp
// UTDGameplayAbilityBase - Project base class
class MYGAME_API UTDGameplayAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UTDGameplayAbilityBase();

protected:
    // Shared activation tag handling
    UPROPERTY(EditDefaultsOnly, Category = "Activation")
    FGameplayTagContainer ActivationRequiredTags;
    
    UPROPERTY(EditDefaultsOnly, Category = "Activation")  
    FGameplayTagContainer ActivationBlockedTags;
    
    // Default input binding
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    int32 AbilityInputID = -1;
};
```

### Blueprint Implementation
Create Blueprint child classes of `UTDGameplayAbilityBase`:
- Set cost/cooldown GameplayEffect references
- Configure activation tags and input bindings
- Implement ability logic using Blueprint nodes
- Use ability tasks for complex sequences

## Debug and Testing

### Console Commands
```
showdebug abilitysystem
```
Shows active abilities, cooldowns, and ability-specific debug info.

### Common Debug Outputs
```cpp
// In ability implementation
UE_LOG(LogTemp, Warning, TEXT("Ability %s activated by %s"), 
    *GetClass()->GetName(), *GetActorInfo().PlayerController->GetName());

#if !UE_BUILD_SHIPPING
GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
    FString::Printf(TEXT("Ability Cost: %.1f Mana"), ManaCost));
#endif
```

### Testing Checklist
- [ ] Ability grants correctly to target ASC
- [ ] Activation checks work (tags, costs, cooldowns)
- [ ] Server/client prediction behaves consistently
- [ ] Cost and cooldown effects apply properly
- [ ] Ability can be interrupted/canceled as expected
- [ ] Animation/audio cues trigger correctly
- [ ] Network replication maintains ability state

## Common Pitfalls

### 1. **Granting Without ActorInfo**
```cpp
// ❌ Wrong - ASC not initialized
ASC->GiveAbility(AbilitySpec); // May crash or fail silently

// ✅ Correct - After InitAbilityActorInfo
ASC->InitAbilityActorInfo(Owner, Avatar);
ASC->GiveAbility(AbilitySpec);
```

### 2. **Missing Authority Checks**
```cpp
// ❌ Wrong - Applies on all clients
CommitAbility(Handle, ActorInfo, ActivationInfo);

// ✅ Correct - Server authoritative
if (HasAuthority(&ActivationInfo))
{
    CommitAbility(Handle, ActorInfo, ActivationInfo);
}
```

### 3. **Ability Task Cleanup**
```cpp
// ❌ Wrong - Tasks may leak or cause crashes
UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, 5.0f);
// Ability ends before task completes

// ✅ Correct - Override EndAbility to clean up
void UMyAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // Tasks automatically clean up when ability ends
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

### 4. **Input ID Conflicts**
Multiple abilities with the same InputID can cause activation conflicts. Use unique IDs or implement input routing logic.

## Implementation Checklist

- [ ] Create base ability class (`UTDGameplayAbilityBase`)
- [ ] Define cost and cooldown GameplayEffects
- [ ] Configure activation and blocking tags
- [ ] Set appropriate net execution policy
- [ ] Implement ability-specific logic
- [ ] Add ability to character startup grants
- [ ] Test activation, execution, and cancellation
- [ ] Verify network behavior in multiplayer
- [ ] Add debug output for development builds

## See Also

- [Base Ability and Startup Grant](base-ability-and-startup-grant.md)
- [Gameplay Effects](../gameplay-effects.md)
- [Gameplay Tags](../gameplay-tags.md)
- [Replication & Multiplayer](../../architecture/replication-and-multiplayer.md)
- [GAS Attribute Callbacks](../cheatsheets/gas-attribute-callbacks.md)