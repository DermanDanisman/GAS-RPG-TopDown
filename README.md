# GAS-RPG-TopDown

A UE5 top‑down RPG project focused on learning and applying the Gameplay Ability System (GAS). The repo includes a living documentation set and a companion plugin design (WIP) to streamline reusing common GAS patterns in other projects.

> Docs are living and will evolve as the course progresses. If you spot anything outdated or unclear, please open a "Docs Update" issue or a PR.

## Highlights

- GAS‑first architecture:
  - Ability System Component (ASC)
  - Attribute Set with accessor macros
  - Gameplay Effects (Instant, Duration, Infinite, Periodic, Stacking)
  - Gameplay Tags and routing of effect asset tags to UI
- Multiplayer‑ready defaults (Mixed replication for Player ASC, Minimal for AI)
- MVC‑style UI with a Widget Controller broadcasting values to widgets via delegates
- Reusable "Effect Actor" pattern for pickups and areas (Instant/Duration/Infinite)

## Documentation

Start here: docs/README.md

Key topics:
- Setup: docs/setup.md
- Architecture: docs/architecture.md
- Attributes & Accessors: docs/attributes-and-accessors.md
- UI & Widget Controller: docs/ui-widget-controller.md
- Gameplay Effects: docs/gameplay-effects.md
- Replication & Multiplayer: docs/replication-and-multiplayer.md
- Gameplay Tags: docs/gameplay-tags.md
- Debugging & Tools: docs/debugging-and-tools.md
- Course notes mapping: docs/course-notes.md
- Plugin design (WIP): docs/plugin/overview.md, docs/plugin/api.md

## Quick Start

1) Enable plugins:
   - GameplayAbilities, GameplayTags, GameplayTasks
2) Build modules (YourProject.Build.cs):
```csharp
PrivateDependencyModuleNames.AddRange(new string[] {
  "GameplayAbilities", "GameplayTags", "GameplayTasks"
});
```
3) Player architecture:
   - PlayerState owns ASC + AttributeSet (NetUpdateFrequency ≈ 100; ASC ReplicationMode = Mixed)
   - Enemies: ASC + AttributeSet on Character (ReplicationMode = Minimal)
4) Initialize ASC actor info once Owner/Avatar are valid.
5) HUD/UI: create Overlay Widget + Overlay Widget Controller; call SetWidgetController() then BroadcastInitialValues().
6) Verify with console: `showdebug abilitysystem`.

## Patterns Used

- UI MVC with Widget Controller
  - Controller holds PlayerController, PlayerState, ASC, AttributeSet; broadcasts initial and reactive values via delegates.
  - Widgets bind to controller delegates in `WidgetControllerSet` and remain display‑only.
- Effect Actor
  - Data‑driven Blueprint/C++ actor applying Instant/Duration/Infinite effects on overlap. Supports policies for apply/remove and periodic ticking.

## Debugging

- Console: `showdebug abilitysystem` to inspect Owner/Avatar, attributes, and tags.
- Quick logs: `GEngine->AddOnScreenDebugMessage` or UE_LOG.

## Contributing

See CONTRIBUTING.md. Small, incremental doc PRs are welcome.

## License

Add a license appropriate for your project.
