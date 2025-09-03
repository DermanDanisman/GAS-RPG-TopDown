# Ability Tags and Policies

Last updated: 2024-12-19

## Purpose and Quick Reference

Ability-level tag containers and policies on UGameplayAbility control when abilities can activate, what they cancel, and how they behave in networked environments. This page provides actionable guidance for configuring these systems effectively.

## Ability Tags

**Purpose:** Identity tags that describe what this ability is.

**Usage:**
- Applied to the ASC while the ability is active
- Used by other abilities for cancel/block logic
- Help categorize abilities for external systems

**Example tags:**
- `Ability.State.Casting` - Ability is in casting state
- `Ability.Type.Offensive` - This is an offensive ability
- `Status.Channeling` - Ability requires channeling

```cpp
// In your base ability class
UPROPERTY(EditDefaultsOnly, Category = "Tags")
FGameplayTagContainer AbilityTags;
```

## Cancel Abilities With Tag

**Purpose:** Automatically cancel other abilities when this ability activates.

**Common use cases:**
- Movement abilities cancel casting
- New attacks cancel previous attacks
- Emergency abilities cancel everything

**Configuration:**
- Set `CancelAbilitiesWithTag` in your ability
- Tags are checked against other abilities' `AbilityTags`
- Canceled abilities call `CancelAbility()` immediately

**Example:**
```cpp
// Movement ability cancels all casting
CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.State.Casting"));
```

## Block Abilities With Tag

**Purpose:** Prevent abilities from activating while this ability is active.

**Difference from Cancel:** Block prevents activation; Cancel stops running abilities.

**Common patterns:**
- Stunned state blocks all abilities
- Casting blocks movement abilities
- Channeling blocks other offensive abilities

**Example tags to block:**
- `Ability.Type.Movement` - Block while casting
- `Ability.Input` - Block all input-driven abilities when stunned

```cpp
// Casting ability blocks movement
BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Type.Movement"));
```

## Activation Owned Tags

**Purpose:** Tags granted to the ASC while this ability is active.

**Key benefits:**
- Other systems can react to ability state
- Enables tag-based ability interactions
- Supports complex ability combinations

**Lifecycle:**
- Applied when ability activates
- Removed when ability ends (success or failure)
- Automatically handled by GAS

**Example:**
```cpp
// Grant casting state while active
ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Casting"));
ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Stunned"));
```

## Activation Required/Blocked Tags

**Required Tags:** ASC must have ALL of these tags to activate.
**Blocked Tags:** ASC cannot have ANY of these tags to activate.

**Common patterns:**
- Require `Status.Alive` for most abilities  
- Block on `Status.Stunned` or `Status.Silenced`
- Require weapon-specific tags for weapon abilities

**Example:**
```cpp
// Require alive, block if stunned or silenced
ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Alive"));
ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Stunned"));
ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Silenced"));
```

## Source Required/Blocked Tags

**Purpose:** Check tags on the ability's source (usually the activating character).

**Usage:** Similar to Activation tags, but explicitly checks the source ASC.
**Difference:** More explicit control over which ASC to check in multi-character scenarios.

**When to use:** 
- Multi-character abilities (summons, pets)
- Abilities that affect multiple targets
- Complex interaction scenarios

## Target Required/Blocked Tags

**Purpose:** Validate tags on the ability's target before activation.

**Common uses:**
- Healing abilities require `Status.Alive`
- Buff abilities blocked by `Status.Immune`
- Damage abilities blocked by `Status.Invulnerable`

**Example:**
```cpp
// Healing spell requires living target
TargetRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Alive"));

// Damage spell blocked by immunity
TargetBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Invulnerable"));
```

## Input: ReplicateInputDirectly

**Recommendation:** **Don't use ReplicateInputDirectly = true**

**Why to avoid:**
- Bypasses GAS prediction system
- Can cause input desync in multiplayer
- Reduces client responsiveness
- Makes debugging harder

**Better approach:**
- Use generic replicated events via gameplay events
- Let GAS handle input prediction naturally
- Use `WaitGameplayEvent` tasks for custom triggers

**Example:**
```cpp
// ❌ Avoid this
ReplicateInputDirectly = true;

// ✅ Use this instead
UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
    this, FGameplayTag::RequestGameplayTag("Input.Confirm"));
```

## Advanced: Instancing Policy

Controls how many instances of this ability can exist.

**Options:**
- `NonInstanced` (Default) - One instance per ability spec
- `InstancedPerActor` - One instance per actor
- `InstancedPerExecution` - New instance each activation

**Selection guidance:**
- **NonInstanced:** Most abilities (attacks, spells)
- **InstancedPerActor:** Passive abilities, toggles
- **InstancedPerExecution:** Complex abilities with overlapping activations

```cpp
// For most abilities
InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

// For toggleable abilities
InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
```

## Advanced: Net Execution Policy

Controls where and how the ability executes in multiplayer.

**Options and Recommendations:**

**LocalPredicted (Recommended for most abilities):**
- Executes locally immediately, then on server
- Best user experience with lag compensation
- Use for: attacks, movement, most interactive abilities

**ServerOnly:**
- Only executes on server
- Use for: admin abilities, cheat detection, critical logic

**LocalOnly:**  
- Only executes locally, never replicates
- Use for: UI effects, local feedback, cosmetic abilities

**ServerInitiated:**
- Server decides when to execute, sends to clients
- Use for: AI abilities, environmental triggers

```cpp
// Most player abilities
NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

// Admin/cheat abilities  
NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
```

## Advanced: ServerRespectsRemoteAbilityCancellation

**Recommendation:** **Keep as false (default)**

**Why to avoid enabling:**
- Allows clients to cancel server-side abilities
- Opens potential for cheating/exploitation  
- Can cause desync between client and server state
- Server should maintain authority over ability lifecycle

**When it might be needed:**
- Specific network architectures
- Custom prediction systems
- Only enable with careful consideration and testing

## Replication Policy

**Note:** Generally no need to change from default.

**Key points:**
- Abilities don't run on simulated proxies by default
- Use GameplayEffects for stat changes that need replication
- Use GameplayCues for visual/audio effects on remote clients
- Ability logic stays on server and owning client

**For remote visuals:**
- Apply replicated GameplayEffects with appropriate cues
- Trigger GameplayCues directly for immediate feedback
- Let GAS handle the networking automatically

## Costs, Cooldowns, and Triggers

**Related documentation:**
- [Gameplay Effects](../gameplay-effects.md) - For implementing costs and cooldowns
- [Base Ability and Startup Grant](base-ability-and-startup-grant.md) - For ability setup patterns
- [GAS Attribute Callbacks](../cheatsheets/gas-attribute-callbacks.md) - For cost validation

**Cost implementation:**
- Create GameplayEffect for mana/stamina costs
- Set as `CostGameplayEffectClass` on ability
- Use `CheckCost()` and `CommitCost()` for validation

**Cooldown implementation:**
- Create GameplayEffect with cooldown tag
- Set as `CooldownGameplayEffectClass` on ability  
- Use `CheckCooldown()` and `CommitCooldown()` for validation

## Testing Checklist and Pitfalls

### Pre-Activation Testing
- [ ] Verify activation requirements/blocks work in all game states
- [ ] Test ability conflicts (cancel/block interactions)
- [ ] Validate cost/cooldown checking
- [ ] Test input binding and triggering

### Execution Testing  
- [ ] Verify ability performs intended effects
- [ ] Test ability tasks and their cleanup
- [ ] Validate target acquisition and filtering
- [ ] Check animation and timing integration

### Network Testing
- [ ] Test in multiplayer with various latency conditions
- [ ] Verify prediction works correctly (LocalPredicted abilities)
- [ ] Check that server maintains authority
- [ ] Test ability cancellation across network

### Common Pitfalls

**Tag typos:** Use centralized tag management ([Gameplay Tags](../gameplay-tags.md))

**Tag hierarchy confusion:**
```cpp
// ❌ Blocks too broadly
BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability"));

// ✅ Specific blocking
BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Type.Movement"));
```

**Forgetting to commit costs:**
```cpp
// ❌ Check cost but don't commit
if (!CheckCost()) return;
// Ability continues without paying cost

// ✅ Commit after successful checks
if (!CommitAbility()) return;
```

**Input ID conflicts:** Ensure unique InputIDs across all abilities

**Task cleanup:** Override `EndAbility()` if using custom cleanup logic

**Network policy misuse:** Don't use ServerOnly for responsive player abilities

## See Also

- [Abilities Overview](overview.md)
- [Base Ability and Startup Grant](base-ability-and-startup-grant.md)
- [Gameplay Effects](../gameplay-effects.md)
- [Gameplay Tags](../gameplay-tags.md)
- [Replication & Multiplayer](../../architecture/replication-and-multiplayer.md)
- [GAS Attribute Callbacks](../cheatsheets/gas-attribute-callbacks.md)