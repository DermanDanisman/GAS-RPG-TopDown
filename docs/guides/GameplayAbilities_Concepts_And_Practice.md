# Gameplay Abilities in GAS — Concepts and Practice (Deep Dive)

Last updated: 2025-08-31

This lesson is a practical, step-by-step guide to Unreal's Gameplay Ability System (GAS) abilities. We'll demystify abilities, specs, activation, replication/prediction, cost and cooldown, ability tasks, and common pitfalls. You'll leave with mental models, runnable examples, and a checklist you can use on every new ability.

Related:
- Abilities Overview: ../gas/abilities/overview.md
- Base Ability + Startup Grant: ../gas/abilities/base-ability-and-startup-grant.md
- Ability Tags & Policies: ../gas/abilities/ability-tags-and-policies.md
- Gameplay Effects — Guide: ../gas/gameplay-effects.md
- Replication & Multiplayer: ../architecture/replication-and-multiplayer.md
- GAS Attribute Callbacks (cheatsheet): ../gas/cheatsheets/gas-attribute-callbacks.md

---

## 1) What is a Gameplay Ability?

- A Gameplay Ability is a class derived from UGameplayAbility that defines an action/skill and the conditions under which it can be used.
- Abilities are instance-driven and can run asynchronously. They:
  - Can start at some moment in time (activation),
  - Perform multi-stage tasks (often with Ability Tasks),
  - Potentially branch, apply Gameplay Effects (GEs), and trigger Gameplay Cues,
  - End themselves or be canceled externally,
  - Support replication and (optionally) client-side prediction.

Think of an ability as a small, self-contained "program" that runs in response to a trigger and interacts with GAS primitives (attributes, effects, tags, cues).

---

## 2) The Activation Lifecycle (Timeline)

High-level timeline and data objects:

1) Grant → Spec:
   - The Ability System Component (ASC) is "granted" an ability class on the server.
   - GAS creates an FGameplayAbilitySpec that stores:
     - The ability class,
     - Current level,
     - Dynamic/runtime data (e.g., input bindings, cooldown state).
   - This spec replicates to the owning client.

2) Attempt to Activate:
   - Activation may be requested by input, AI logic, gameplay events, or code.
   - GAS evaluates CanActivateAbility (cost, cooldown, tags).
   - If approved, GAS instantiates or reuses the ability per the instancing policy.

3) Execute (Run):
   - Ability's ActivateAbility runs.
   - Ability can:
     - Apply Gameplay Effects (cost, buff/debuff, cooldown),
     - Start Ability Tasks (timers, montages, waits, events),
     - Drive animations, VFX, SFX, projectiles, traces, etc.

4) End or Cancel:
   - EndAbility: natural completion (optionally after async tasks).
   - CancelAbility: external cancel (e.g., stun/interruption).
   - Cleanup happens; cooldowns may continue tracking via Gameplay Effects.

5) Potential Re-activation:
   - After cooldown expires and conditions are met, the ability can be activated again.

---

## 3) Replication and Prediction (Practical Overview)

- Server Authoritative Granting:
  - Abilities are typically granted on the server. The resulting ability specs replicate to the owning client.
- Client Activation Requests:
  - Common pattern: client requests activation (input), server validates (CanActivateAbility), then activates.
- Prediction:
  - GAS supports client prediction for responsiveness. Keep it high-level at first:
    - Your client can "predict" activation (especially for instant abilities),
    - The server confirms and corrects if necessary.
- Practical Guidelines:
  - Grant on the server.
  - Let the ASC handle replication of the specs.
  - Keep your early prototypes server-validated; add prediction once you're comfortable.

---

## 4) Cost and Cooldown — Where They Live

- Costs and Cooldowns are best modeled using Gameplay Effects:
  - Cost GE: immediately removes resources (e.g., Mana) at activation.
  - Cooldown GE: applies a duration-based state that prevents re-activation.
- Typical flow in C++ abilities:
  - In ActivateAbility: call CommitAbility() which:
    - Runs CanActivate checks (again),
    - Applies cost and cooldown Gameplay Effects if configured on the ability,
    - Returns success/failure.
- Alternate checks:
  - Override CanActivateAbility to add custom logic.
  - In Blueprints, use ability nodes that map to these C++ paths.

Example (C++ pseudo):

```cpp
void UMyGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                            const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo,
                            const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility()) // applies cost & cooldown if configured
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Do the main work, optionally via ability tasks...
}
```

---

## 5) Ability Tasks — Asynchronous Building Blocks

- Ability Tasks are classes derived from UAbilityTask that run asynchronous work and emit delegates.
- In Blueprint, they show up as special nodes with multiple output pins (mapped to delegates).
- Common patterns:
  - PlayMontageAndWait: plays an animation montage and outputs on BlendOut/Interrupted/Cancelled.
  - WaitGameplayEvent: listens for a gameplay event tag and outputs when received.
  - WaitDelay / SetTimer: delay execution without blocking.
- Control flow:
  - "Something happens" → task fires delegate → Blueprint execution flows through the corresponding pin.

BP mental model:
- Nodes like "PlayMontageAndWait" or "WaitGameplayEvent" are essentially "subscribe-and-continue" operations. They keep the ability alive until their condition is met.

---

## 6) Example Walk-through: Simple Fireball

Let's walk through a concrete example:

### Setup
1. Create a Blueprint ability derived from UTDGameplayAbilityBase
2. Configure cost GameplayEffect (e.g., -25 Mana)
3. Configure cooldown GameplayEffect (e.g., 3 second duration with "Cooldown.Fireball" tag)

### Blueprint Implementation
```
Event ActivateAbility
 ↓
Check CommitAbility
 ↓ (Success)
PlayMontageAndWait (cast animation)
 ↓ (On Completed)
Spawn Projectile
 ↓
Apply Damage GameplayEffect to Target
 ↓
EndAbility
```

### What Happens Under the Hood
1. Player presses input → ASC receives activation request
2. Server validates: mana >= 25, no "Cooldown.Fireball" tag present
3. CommitAbility applies cost (-25 mana) and cooldown (3s block)
4. Ability plays animation, waits for completion
5. Projectile spawns, travels, applies damage effect
6. Ability ends naturally

### Projectile Implementation Details

The fireball projectile follows the patterns described in [Projectiles in GAS Abilities](../gas/abilities/projectiles.md):

- **Spawning**: Server-authoritative spawning after cast animation completes
- **Movement**: Physics-based projectile movement with collision detection
- **Damage**: Applies instant fire damage GameplayEffect on hit
- **VFX**: Fire trail during flight, explosion particle effect on impact
- **Network**: Projectile replicates to all clients for visual consistency

For detailed implementation examples, see the [Projectiles documentation](../gas/abilities/projectiles.md).

---

## 7) Debugging Your Abilities

### Console Commands
```
showdebug abilitysystem
```
Shows active abilities, granted abilities, cooldowns, and attribute values.

### Common Debug Patterns
```cpp
// In C++ abilities
UE_LOG(LogTemp, Warning, TEXT("Ability %s: CanActivate = %s"), 
    *GetClass()->GetName(), CanActivateAbility() ? TEXT("True") : TEXT("False"));

#if !UE_BUILD_SHIPPING
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
        FString::Printf(TEXT("Fireball cooldown remaining: %.1f"), GetCooldownTimeRemaining()));
}
#endif
```

### Blueprint Debug
- Use Print String nodes liberally during development
- Check ability's "Can Activate" output before trying to activate
- Monitor attribute changes via the Widget Controller delegate system

---

## 8) Common Pitfalls and Solutions

### Pitfall 1: Forgetting CommitAbility
**Problem:** Ability activates without consuming resources or applying cooldown.
**Solution:** Always call CommitAbility (or CommitCost/CommitCooldown separately) early in ActivateAbility.

### Pitfall 2: Task Cleanup Issues
**Problem:** Ability Tasks continue running after ability ends, causing crashes or leaks.
**Solution:** Tasks automatically clean up when ability ends. Don't manually destroy them.

### Pitfall 3: Replication Confusion
**Problem:** Ability behaves differently on server vs client.
**Solution:** 
- Grant abilities on server only
- Use server-validated activation for critical gameplay
- Test in multiplayer early and often

### Pitfall 4: Input ID Conflicts
**Problem:** Multiple abilities respond to the same input.
**Solution:** Use unique InputID values or implement input routing logic in your base ability class.

### Pitfall 5: Tag Policy Misunderstandings
**Problem:** Abilities don't activate when expected due to tag requirements.
**Solution:** Use `showdebug abilitysystem` to inspect current tags and ability states.

---

## 9) Advanced Patterns

### Channeled Abilities
Use WaitDelay in a loop with periodic effects:
```
ActivateAbility → CommitAbility → Start Channel Loop
 ↓
WaitDelay (0.5s) → Apply Periodic Effect → Check if still channeling
 ↓ (Loop continues)
Until: Input released OR interrupted OR out of resources
 ↓
EndAbility
```

### Combo Abilities
Chain multiple abilities using WaitGameplayEvent:
```
Ability A → Apply Effect with "ComboWindow.A" tag → EndAbility
Ability B → Check for "ComboWindow.A" tag → Enhanced damage if present
```

### Conditional Branching
Use gameplay tags to create branching ability logic:
```cpp
if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(
    FGameplayTag::RequestGameplayTag("State.Empowered")))
{
    // Enhanced version
    ApplyGameplayEffectToOwner(EmpoweredFireballEffect);
}
else
{
    // Normal version  
    ApplyGameplayEffectToOwner(NormalFireballEffect);
}
```

---

## 10) Testing Checklist

Before considering an ability "done", verify:

- [ ] Ability grants correctly to target ASC
- [ ] CanActivateAbility properly checks costs, cooldowns, and tags
- [ ] CommitAbility applies cost and cooldown effects
- [ ] Ability logic executes as expected (damage, buffs, etc.)
- [ ] Ability ends cleanly (no hanging tasks or references)
- [ ] Cooldown prevents re-activation for the expected duration
- [ ] Server/client behavior is consistent in multiplayer
- [ ] Animation and audio cues trigger correctly
- [ ] Debug output helps diagnose issues during development
- [ ] Edge cases handled (low resources, interruption, target death)

---

## 11) FAQ

**Q: Should I create abilities in C++ or Blueprint?**
A: Start with Blueprint for rapid prototyping. Move to C++ for complex logic, performance-critical abilities, or shared functionality across many abilities.

**Q: How do I make an ability that doesn't consume resources?**
A: Don't set CostGameplayEffectClass, or override CanActivateAbility to bypass cost checks.

**Q: Can abilities activate other abilities?**
A: Yes, use TryActivateAbilityByClass or send a GameplayEvent that triggers another ability.

**Q: How do I interrupt all abilities when a character dies?**
A: Call CancelAllAbilities on the ASC, typically in the character's death logic.

**Q: What's the difference between EndAbility and CancelAbility?**
A: EndAbility is for natural completion. CancelAbility is for external interruption (stun, death, etc.).

---

## Next Steps

1. **Implement Your First Ability**: Create a simple instant ability (like a heal or buff) using the patterns above.
2. **Add Ability Tasks**: Experiment with PlayMontageAndWait and WaitDelay for more complex sequences.
3. **Test Multiplayer**: Run your abilities in a multiplayer session to understand replication behavior.
4. **Explore Advanced Patterns**: Try channeled abilities, combos, or conditional logic based on gameplay tags.
5. **Optimize Performance**: Profile your abilities and move performance-critical logic to C++ as needed.

This deep dive should give you the foundation to confidently create, debug, and maintain abilities in your GAS project. Remember: start simple, test frequently, and iterate based on gameplay needs.