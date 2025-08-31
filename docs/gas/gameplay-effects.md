# Gameplay Effects — Guide

Last updated: 2025-08-31

Practical reference for authoring and using Gameplay Effects (UGameplayEffect) for costs, cooldowns, buffs/debuffs, periodic effects, and attribute math.

Related:
- Abilities Overview: ./abilities/overview.md
- Ability Tags & Policies: ./abilities/ability-tags-and-policies.md
- Attribute Clamping: ./attributes/attribute-clamping.md
- GAS Attribute Callbacks (cheatsheet): ./cheatsheets/gas-attribute-callbacks.md
- Derived Attributes and Calculations: ./attributes/custom-calculations.md
- Replication & Multiplayer: ../architecture/replication-and-multiplayer.md
- Gameplay Tags: ./gameplay-tags.md

## Core concepts

- Effect kinds
  - Instant: applies immediately (e.g., cost payments, direct damage).
  - Duration: applies for a fixed time (e.g., cooldowns, short buffs).
  - Infinite: applies until removed (e.g., passives, toggles).
- Modifiers
  - Additive, Multiplicative, Override, and custom via MMCs (Magnitude Calculation classes).
  - Prefer MMCs for derived/conditional math; keep raw effects simple.
- Executions
  - GameplayEffectExecutionCalculations are for complex multi-attribute calculations (e.g., damage formulas using Attack, Defense, Crit).
  - Use executions when multiple attributes and tags contribute to the result; keep "leaf" effects minimal.
- Periodic execution
  - Duration/Infinite effects can tick periodically (heal-over-time, damage-over-time). Each tick can apply modifiers or run an execution.
- Stacking (high level)
  - Configure stack limit, stacking behavior (by Source vs Target), and refresh/expiration policy. Be explicit; don't rely on defaults for combat-critical effects.
- Tags
  - GrantedTags identify states (e.g., Cooldown tags), OngoingTagRequirements gate persistence, and ApplicationTagRequirements guard initial application.
  - GrantedApplicationImmunityTags let you declare immunities (e.g., immune to Status.Stun).

Tip: For attribute math and order-of-operations nuances, pair this page with:
- Attribute Clamping for safe min/max guarding and preventing under/overflow.
- Attribute Callbacks cheatsheet for OnAttributeChange, pre/post-change hooks, and UI updates.

## Costs pattern (Instant)

Goal: Subtract resources safely, reflect changes in UI, and fail activation if unaffordable.

- Author a small Instant GE (Cost_Mana_Fireball) with a negative additive modifier to Mana.
- Ensure attributes involved are clamped; see Attribute Clamping to avoid going below zero.
- Assign as CostGameplayEffectClass on the ability; CommitAbility() will apply it when committing.

C++ example:
```cpp
// Ability class properties
UPROPERTY(EditDefaultsOnly, Category="Costs")
TSubclassOf<UGameplayEffect> CostGameplayEffectClass;

// In ActivateAbility override
if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
{
    EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    return false;
}
```

Blueprint equivalents:
- In the ability, set Cost Gameplay Effect Class, then call Commit Ability right after validation.
- Use Attribute change callbacks (cheatsheet) to drive UI instead of polling.

## Cooldowns pattern (Duration + tag gate)

Goal: Block re-activation while a cooldown effect is active by using tags.

- Author a Duration GE (CD_Fireball_3s) that Grants tag Cooldown.Fireball for 3 seconds.
- Set this GE as CooldownGameplayEffectClass in the ability.
- GAS checks owner for cooldown tags during CanActivate/CommitAbility; no extra code required.

Notes:
- Keep cooldown tags scoped: Cooldown.[AbilityName] to avoid accidental cross-blocking.
- For multiple ranks/levels, either parameterize the magnitude (MMC/SetByCaller) or create per-rank assets and assign per ability level.
- To query remaining cooldown in UI, aggregate by tag with ASC helpers and avoid tight polling.

## CommitAbility cookbook

Use these patterns to balance responsiveness vs. correctness:

1) Commit immediately (default)
- Call CommitAbility() at the start of ActivateAbility after validations.
- Use when cost/cooldown should always be paid on press.
Pros: Simple; server truth consistent. Cons: Cost paid even if later miss/fail.

2) Commit on hit/target confirm
- Begin predicted flow (e.g., PlayMontageAndWait, WaitTargetData).
- After confirm (hit result valid), call CommitAbility() then apply effects.
Pros: Avoids paying costs on whiff. Cons: More branching; ensure prediction paths are handled.

3) Split commit (partial cost now, rest on confirm)
- Apply a small "reservation" cost up front and the remainder on confirm.
- Useful for long wind-ups or channeled abilities to deter spam while still being fair.

4) Graceful fail
- If CommitAbility() returns false, immediately EndAbility(..., bWasCancelled=true).
- Log with reason (insufficient resource, active cooldown) for debugging.

5) SetByCaller and dynamic magnitudes
- When cost/cooldown depends on context, pass SetByCaller values before CommitAbility().
- Validate inputs server-side; clamp magnitudes to sane ranges (see Attribute Clamping).

Blueprint tips:
- Keep CommitAbility close to activation entry or the confirm node; document the choice with a comment.
- For confirm flows, guard duplicates and ensure EndAbility fires on all exit paths.

## Effect application API (C++ and Blueprint)

C++ (owner/self):
```cpp
FGameplayEffectContextHandle Ctx = MakeEffectContext(Handle, ActorInfo);
FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(MyEffectClass, AbilityLevel, Ctx);
// Optional: SetByCaller variables
Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Damage")), DamageValue);
ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
```

C++ (target):
```cpp
FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(MyEffectClass, AbilityLevel, MakeEffectContext(Handle, ActorInfo));
ASC_Target->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
```

Blueprint equivalents:
- Make Outgoing GE Spec → Set By Caller Magnitude (optional) → Apply GE to Self/Target.

Notes:
- Always prefer applying effects on the server (authoritative). For predicted abilities, let GAS handle prediction keys within ability execution.
- Use GameplayEffectContext to pass instigator/causer, hit results, or source object.

## Tag gates and immunities

- ApplicationTagRequirements: required/blocked owner tags for effect to apply initially.
- OngoingTagRequirements: required/blocked tags for the effect to persist; if broken, the effect is removed.
- GrantedApplicationImmunityTags: prevents application of incoming effects matching tags (e.g., Status.Immune.Stun).
- RemoveEffectsWithTags: bulk-remove using tag queries (e.g., clear all Status.Poison effects).

Pattern: Use clear categories
- Status.* for crowd-control and states (Stunned, Rooted, Burning)
- Cooldown.* for ability-specific cooldowns
- Buff.* / Debuff.* for generic buffs

## Executions vs MMCs

- MMCs compute a single magnitude for a modifier (great for "+X% damage from Power").
- Executions can read multiple attributes/tags from source and target, run full formulas, and output multiple modifiers at once (great for combat formulas).
- Prefer MMCs for simple, composable math; use Executions for complex, multi-input results.

## Periodic effects

- Periodic Frequency controls how often the effect ticks.
- Each tick may re-apply modifiers or invoke an Execution to recompute values.
- Document whether the first tick happens immediately or after one period and align visuals.

## Removal and expiry

- Duration effects auto-expire; Infinite effects require manual removal (RemoveActiveGameplayEffect...).
- Use tags (OngoingTagRequirements/RemoveGameplayEffectWithTags) for bulk cleanup.
- For canceling on crowd control, pair ability-owned state tags (Ability.State.Casting) with Cancel/Block tag containers (see Ability Tags & Policies).

## Gameplay Cues integration

- Effects can grant GameplayCue.* tags to drive audio/visual feedback on apply/remove and during loops.
- Keep cue tags separate from state tags (e.g., Cue.Status.Burning alongside Status.Burning).

## Replication, authority, and prediction

- Apply effects on the server; abilities can be LocalPredicted for responsiveness.
- For predicted applications, the ability's prediction key ensures reconciliation; avoid manual client-only effect application.
- Effects and their tags replicate to the owning client; simulated proxies should rely on cues/animations rather than executing ability logic.

## Debugging

- showdebug abilitysystem for active effects, tags, and aggregators.
- Gameplay Debugger (apostrophe ') → AbilitySystem for live effect/tag view.
- Log on application and removal; include GrantedTags and remaining time for cooldowns.
- Add temporary cues for clarity while tuning (e.g., Cue.Debug.CooldownApplied).

## Worked examples

1) Fireball — cost + cooldown
- Cost_Mana_Fireball (Instant, -30 Mana)
- CD_Fireball_3s (Duration 3s, Grants Cooldown.Fireball)
- Ability commits immediately after validation; UI subscribes to Mana and cooldown changes.

2) Burn DoT — periodic damage with cue
- GE_Burn_DoT (Duration 6s, Period 1s, Grants Status.Burning and Cue.Status.Burning)
- Execution reads Power and Target.Resistance to compute per-tick damage.
- OngoingTagRequirements: removed if Status.Wet is present (example interaction).

3) Shield — infinite with max cap
- GE_Shield (Infinite, Grants Buff.Shield, modifier adds to Shield attribute)
- Attribute clamped via project's clamping rules to prevent overflow.
- Removed on Status.Dispel via RemoveEffectsWithTags.

## Testing checklist

- [ ] Costs fail activation cleanly when unaffordable; no negative resources (see Attribute Clamping).
- [ ] Cooldowns grant the correct Cooldown.* tags and block re-activation.
- [ ] Periodic effects tick at intended frequency and stop on expiry.
- [ ] Immunity tags prevent application as designed; logs confirm.
- [ ] SetByCaller magnitudes are clamped and validated.
- [ ] Cues play once on apply, loop correctly, and stop on remove.
- [ ] No UI polling; attribute/cooldown displays subscribe to callbacks.

## Common pitfalls

- Forgetting to configure GrantedTags on cooldown GEs — the ability won't see its own cooldown.
- Relying on raw additive/multiplicative mixes without MMCs — brittle across balance changes.
- Not clamping attributes — negative resources or overshoot during bursts.
- Applying effects client-side for cosmetics — causes desync; use Gameplay Cues instead.
- UI polling the ASC directly — use callbacks to avoid stale data.