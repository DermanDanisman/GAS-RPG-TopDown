# GAS-RPG-TopDown

A UE5 top‑down RPG project built around the Gameplay Ability System (GAS). This README includes concise, self‑contained documentation summaries for the core systems in this repo so you don’t need to navigate elsewhere.

If you notice anything outdated or unclear, please open a “Docs Update” issue or submit a small PR.

## Highlights

- GAS‑first architecture with Ability System Component (ASC), Attribute Set, and data‑driven Gameplay Effects.
- Multiplayer‑ready defaults and patterns for Players and AI.
- MVC‑style UI using a Widget Controller that broadcasts values via delegates.
- Reusable Effect Actor pattern for pickups/areas with Instant/Duration/Infinite effects.
- Robust three‑tier attribute clamping to prevent “invisible buffers” and out‑of‑range values.
- Cleaned‑up debugging: UE_LOG diagnostics guarded out of shipping builds.
- Primary attributes established and initialized (STR, DEX, INT, END, VIG).

## Quick Start

1) Enable plugins:
   - GameplayAbilities
   - GameplayTags  
   - GameplayTasks
   - CommonUI (already enabled in the project)
   - ClickToMove (already enabled in the project)
2) Add dependencies in your Build.cs:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] {
     "GameplayAbilities", "GameplayTags", "GameplayTasks"
   });
   ```
3) Recommended architecture:
   - Player: ASC + AttributeSet live on PlayerState. ASC ReplicationMode = Mixed.
   - AI: ASC + AttributeSet live on Character/Pawn. ReplicationMode = Minimal.
4) Initialize ASC ActorInfo after Owner and Avatar are valid.
5) UI flow:
   - Create Overlay Widget and a corresponding Overlay Widget Controller.
   - Inject controller with references (PC, PS, ASC, AttributeSet).
   - Call SetWidgetController() and then BroadcastInitialValues().
   - Widgets bind to controller delegates and remain display‑only.
6) Verify in game:
   - Use the “showdebug abilitysystem” console command to inspect state.

## Attribute Clamping (Deep Summary)

Why three tiers?
- GameplayEffects can modify both base and current attribute values through different pipelines (Instant, Periodic, Duration, Infinite). Clamping at one point is not sufficient to prevent transient overflows or “invisible buffer” accumulation. A layered approach keeps values safe and consistent on both server and clients.

Tiers and responsibilities:
- PreAttributeChange(Attribute, NewValue)
  - Purpose: Clamp visible (current) value before it is applied.
  - Rule: Only clamp NewValue. Do NOT call setters or write to other attributes here.
  - Typical clamps: Health, Mana, Stamina against 0..Max ranges when current values change directly.
- PreAttributeBaseChange(Attribute, NewValue)
  - Purpose: Clamp base value before it is applied by Instant/Periodic effects.
  - Rule: Only clamp NewValue. Do NOT call setters or trigger side‑effects here.
  - Typical clamps: BaseHealth, BaseMana, BaseStamina to valid ranges, base Max attributes to sane bounds.
- PostGameplayEffectExecute(Data)
  - Purpose: Final authoritative clamp and any side‑effects that must run after an effect is executed.
  - Rule: It is safe to set attributes here using setters. This is the last line of defense.
  - Mandated by repository changes:
    - Mana: SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()))
    - Stamina: SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()))
  - Also handle derived reactions (e.g., death triggers on Health <= 0) here.

Max attribute changes:
- When MaxX changes, ensure X remains within [0, MaxX].
- Prefer preserving ratio only when explicitly desired by design; otherwise clamp to the new bounds.
- Apply the final correction in PostGameplayEffectExecute to guarantee authority consistency.

Common pitfalls avoided:
- Calling setters in Pre* callbacks (can recurse/emit events prematurely).
- Relying solely on Pre* to prevent overflow (misses some execution paths).
- Forgetting to clamp after Max attribute adjustments.

Testing checklist:
- Instant, Duration, and Periodic effects cannot push values beyond bounds.
- Reducing Max while X is high clamps X to the new Max in the same frame.
- Server and client remain in sync after heavy effect spam and network latency.
- Death/KO triggers exactly once when Health reaches zero.
- Regeneration and drains never oscillate due to rounding or ordering issues.

## GAS Attribute Callbacks Cheatsheet (Summary)

- PreAttributeChange(Attribute, NewValue)
  - Fires before current value changes.
  - Safe: Clamp NewValue only. No side‑effects, no setters.
  - Typical use: Keep current within [0, Max].
- PreAttributeBaseChange(Attribute, NewValue)
  - Fires before base value changes (commonly from GE execution or periodic ticks).
  - Safe: Clamp NewValue only. Avoid cascading updates.
  - Typical use: Constrain base stats and base Max attributes.
- PostGameplayEffectExecute(Data)
  - Fires after a GE modifies attributes.
  - Safe: Apply final clamps with setters and trigger reactions.
  - Typical use: Final authority, e.g., Mana/Stamina clamps, death checks, tag/apply effects.
- OnRep_Attribute (clients)
  - Keep client UI/FX in sync; do not perform authority‑only logic here.
- Authority considerations
  - Do not gate clamps behind client‑only code.
  - Keep deterministic logic server‑side; clients mirror for UI.

## UI and Widget Controller (Summary)

- Separation of concerns:
  - Widget Controller owns references (PC, PS, ASC, AttributeSet) and exposes typed delegates for attribute and tag changes.
  - Widgets subscribe to controller delegates and purely render data.
- Initialization flow:
  - SetWidgetController() injects dependencies once ready.
  - BroadcastInitialValues() sends the current snapshot so widgets render immediately.
- Practical tips:
  - Avoid querying ASC directly from widgets.
  - Use weak references or validate before broadcast if lifecycle is dynamic.
  - Add light throttling for rapidly changing attributes to avoid UI spam.

## Gameplay Effects (Summary)

- Supported modes: Instant, Duration, Infinite, Periodic, Stacking with overflow policies.
- Captured attributes: Use scoped modifiers and clear capture definitions to avoid surprises.
- Meta‑attributes: Route aggregated changes through PostGameplayEffectExecute for final decisions.
- Tag‑driven design: Use Gameplay Tags on assets to drive conditional logic and UI adornments.
- Ordering:
  - Base changes land via PreAttributeBaseChange, then PostGameplayEffectExecute finalizes.
  - Current changes touch PreAttributeChange first, PostGameplayEffectExecute last.

## Replication and Multiplayer (Summary)

- Players
  - ASC on PlayerState for consistent lifetime across possession.
  - ReplicationMode = Mixed: reduces bandwidth by replicating only what’s needed for owners vs non‑owners.
- AI
  - ASC on Character/Pawn; ReplicationMode = Minimal: sends only necessary data.
- Initialization
  - Call InitAbilityActorInfo once Owner/Avatar are valid.
  - Ensure abilities are granted on the server; clients receive replicated specs.
- Performance
  - Keep NetUpdateFrequency reasonable; adjust for your project’s needs.
  - Avoid per‑tick heavy logs and large attribute broadcasts.

## Effect Actor Pattern (Summary)

- Purpose
  - Reusable actor that applies effects on overlap or interaction.
- Policies
  - Instant: fire once then optionally destroy.
  - Duration: apply on begin, remove on end/exit.
  - Infinite: apply on begin, remove on exit or timeout.
- Considerations
  - Authority checks for application/removal.
  - Tag gates to prevent double application.
  - Optional cooldowns and per‑actor limits.

## Debugging and Logging (Summary)

- Prefer UE_LOG categories over on‑screen debug for durability and filtering.
- Guard logging out of shipping builds using compile‑time checks.
- Handy tools
  - Console: “showdebug abilitysystem”
  - Visualize attributes, tags, and ability specs in PIE.
- Troubleshooting tips
  - If values drift, verify clamp paths hit: Pre*, PostExecute, and OnRep.
  - Ensure Max changes are followed by a final clamp in PostExecute.
  - Cross‑check authority: server truths must reconcile client displays.

## Primary Attributes (Summary)

- Established and initialized core RPG stats:
  - Strength (STR), Dexterity (DEX), Intelligence (INT), Endurance (END), Vigor (VIG).
- Usage
  - Drive secondary attributes and ability scaling via GameplayEffects.
  - Clamp and validate base values in PreAttributeBaseChange to maintain sane bounds.

## Conventions

- Keep gameplay logic server‑authoritative; mirror to clients for presentation.
- Avoid calling setters in Pre* callbacks; apply final corrections in PostGameplayEffectExecute.
- Keep UI passive; rely on controller broadcasts rather than direct ASC queries.

## Contributing

Small, focused PRs are welcome, especially for docs clarity and examples. Please follow existing patterns and add tests or repro steps for behavior changes.

## License

This project does not currently have a formal license. Please contact the project maintainer for usage permissions.
