# Derived (Secondary) Attributes with GAS

Goal: Make secondary and vital attributes recompute automatically whenever their backing (primary) attributes change — without manual code.

We achieve this with a single infinite Gameplay Effect (GE) that uses Override + AttributeBased magnitudes (no snapshot). Because the GE is infinite and not snapshotted, the aggregator recalculates the magnitude whenever the captured (backing) attributes change. No ticks or timers required.

## Why an Infinite, AttributeBased GE?
- Infinite keeps the effect resident, so the aggregator can always recalculate.
- Override guarantees the current value is set from our formula (rather than adding or multiplying).
- AttributeBased magnitude lets us express the value as:
  value = PostAdd + Coefficient × (PreAdd + CapturedAttribute)
- No Snapshot means the captured attribute is read live at evaluation time (so changes propagate).

## Setup checklist (Editor)
Create an asset, e.g. `GE_DefaultSecondaryAttributes`:
1) Duration Policy: Infinite
2) For each derived attribute (secondary or vital):
   - Modifier Op: Override
   - Magnitude: Attribute Based
   - Attribute To Capture: the backing attribute from the table below
   - Attribute Source: Target
   - Snapshot: false
   - Set PreAdd / Coefficient / PostAdd to match the desired formula

Tip: Periodic is not needed. GAS automatically re-evaluates AttributeBased magnitudes when the backing attribute changes.

## Where/when to apply
- Apply the default primary attributes first (instant GE).
- Then apply the infinite derived-attributes GE.
- Do this after `AbilitySystemComponent->InitAbilityActorInfo(...)`.
- In multiplayer, apply on the server; clients get values via replication.

If you use `TDDefaultAttributeInitComponent`, call its initialize method after ASC init and ensure it applies both primary and derived GEs (primaries first).

## Recommended starter mappings
These are reasonable defaults; tune to your game or move numbers into curves later.

| Attribute               | Formula                               | Backing Attribute      | PreAdd | Coef  | PostAdd |
|-------------------------|---------------------------------------|------------------------|--------|-------|---------|
| Armor                   | 6 + 0.25 × Endurance                  | Endurance              | 0      | 0.25 | 6       |
| Armor Penetration       | 3 + 0.15 × Dexterity                  | Dexterity              | 0      | 0.15 | 3       |
| Block Chance            | 4 + 0.25 × Armor                      | Armor                  | 0      | 0.25 | 4       |
| Critical Hit Chance     | 2 + 0.25 × Armor Penetration          | Armor Penetration      | 0      | 0.25 | 2       |
| Critical Hit Damage     | 5 + 1.5 × Armor Penetration           | Armor Penetration      | 0      | 1.5  | 5       |
| Critical Hit Resistance | 10 + 0.25 × Armor                     | Armor                  | 0      | 0.25 | 10      |
| Health Regeneration     | 1 + 0.1 × Vigor                       | Vigor                  | 0      | 0.1  | 1       |
| Mana Regeneration       | 1 + 0.1 × Intelligence                | Intelligence           | 0      | 0.1  | 1       |
| Stamina Regeneration    | 1 + 0.1 × Endurance                   | Endurance              | 0      | 0.1  | 1       |
| Max Health              | 80 + 2.5 × Vigor                      | Vigor                  | 0      | 2.5  | 80      |
| Max Mana                | 50 + 2.0 × Intelligence               | Intelligence           | 0      | 2.0  | 50      |
| Max Stamina             | 60 + 2.0 × Endurance                  | Endurance              | 0      | 2.0  | 60      |

All rows use: Op = Override, Source = Target, Snapshot = false, GE Duration = Infinite.

## Teachable moments and pitfalls
- Don't snapshot derived attributes: Snapshot caches the captured value once, preventing live updates.
- Don't manually initialize derived BaseValues in the AttributeSet ctor: the GE will set them anyway.
- Initialization order matters: primaries first, then derived.
- Periodic not required: re-evaluation is event-driven by aggregator updates.
- If several rows use `bDestroyOnEffectApplication` semantics elsewhere, consider deferring `Destroy()` until all rows processed.
- Consider clamping current values when Max attributes change (see your AttributeSet's Pre/Post hooks).

## Testing
- Create a simple test GE that adds, e.g., +2 Dexterity.
- Apply it and observe Armor Penetration and Crit Chance rise accordingly.
- Use `ShowDebug AbilitySystem` to verify; plan an Attributes UI panel for richer visualization.

## Scaling with curves
- Move PreAdd/Coefficient/PostAdd into scalable floats (curve tables) over time.
- You can scale with character level or difficulty and keep formulas stable while values grow.

## Appendix: Apply via code (where it belongs)
- Apply your default primaries (instant) and the infinite `GE_DefaultSecondaryAttributes` right after ASC init.
- See `TDDefaultAttributeInitComponent` for a centralized place to do this in game code.

---

This approach mirrors how many RPGs model interdependent stats and keeps logic declarative inside a single GE, while GAS handles re-evaluation for you.