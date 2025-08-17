# GAS-RPG-TopDown

A UE5 top-down RPG project focused on learning and applying the Gameplay Ability System (GAS). The repo also includes a companion plugin design (docs only for now) to streamline reusing common GAS patterns in other projects.

This documentation distills lessons from the Udemy course “Unreal Engine 5 GAS Top-Down RPG” and adapts them into a production-minded structure with code patterns, replication choices, UI architecture, and effect/attribute handling.

## Highlights

- GAS-first architecture with:
  - Ability System Component (ASC)
  - Attribute Set with boilerplate accessors
  - Effects (Instant, Duration, Infinite, Periodic, Stacking)
  - Gameplay Tags and asset tag routing to UI
- Multiplayer-ready replication choices (Mixed for Player, Minimal for AI)
- MVC-style UI using a Widget Controller that broadcasts data to widgets via delegates
- Reusable “Effect Actor” Blueprint/C++ pattern for pickups, areas, and periodic/infinite effects

## Quick Start

1) Enable plugins:
- GameplayAbilities
- GameplayTasks
- GameplayTags

2) Add modules to your .Build.cs:
```csharp
PrivateDependencyModuleNames.AddRange(new string[] {
  "GameplayAbilities",
  "GameplayTags",
  "GameplayTasks"
});
```

3) Player setup:
- Create a PlayerState (e.g., AuraPlayerState)
- Add ASC + AttributeSet to PlayerState (NetUpdateFrequency ~100)
- Init ASC actor info (Owner: PlayerState, Avatar: Character)
- ASC replication mode: Mixed (player), Minimal (AI)

4) UI setup:
- Base UUserWidget (AuraUserWidget) with SetWidgetController()
- Base Widget Controller (AuraWidgetController) with references to PlayerController, PlayerState, ASC, AttributeSet
- HUD initializes and wires Overlay Widget + Controller
- Overlay/Globe widgets bind to controller delegates (OnHealthChanged, OnMaxHealthChanged, etc.)

5) Effects:
- Use Gameplay Effects (GE) for attribute changes
- Prefer specs (MakeOutgoingSpec + ApplyX) and optionally Blueprints (ASC Blueprint Library)
- Use Instant for permanent base changes; Duration/Infinite for temporary current-value changes; Periodic for HoT/DoT; configure stacking

## Repository Layout

- docs/
  - setup.md
  - architecture.md
  - attributes-and-accessors.md
  - ui-widget-controller.md
  - gameplay-effects.md
  - replication-and-multiplayer.md
  - gameplay-tags.md
  - debugging-and-tools.md
  - course-notes.md
  - plugin/
    - overview.md
    - api.md

## Learning Mode

The docs include a course-notes.md mapping your transcripts to practical implementation steps. Use that to plan tasks and cross-reference the why behind each pattern choice.

## License

Add a license appropriate for your project.