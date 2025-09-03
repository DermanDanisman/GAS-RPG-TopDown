# GAS Utility Plugin (Design)

Last updated: 2024-12-19

Goal: package common GAS patterns into a reusable plugin to speed up integration in new projects.

## Included Patterns

- TDAbilitySystemComponent (ASC subclass)
  - Hook: `AbilityActorInfoSet()`
  - Delegate: `FEffectAssetTags` broadcasting effect asset tags on apply
  - Helper: replication mode setup

- TDAttributeSet (AttributeSet base)
  - ATTRIBUTE_ACCESSORS macro and ready-to-use template
  - Common attributes: Health/MaxHealth/Mana/MaxMana
  - Debug utilities

- TDUserWidget (UUserWidget base)
  - `SetWidgetController(UObject*)` (BlueprintCallable)
  - `WidgetControllerSet()` (BlueprintImplementableEvent)

- TDWidgetController (UObject base)
  - BlueprintReadOnly refs: PlayerController, PlayerState, ASC, AttributeSet
  - Virtual `BroadcastInitialValues()`
  - `BindCallbacksToDependencies()` pattern

- TDEffectActor (AActor)
  - TSubclassOf<UGameplayEffect>: Instant/Duration/Infinite
  - Application policy (ApplyOnOverlap/ApplyOnEndOverlap/DoNotApply)
  - Removal policy (RemoveOnEndOverlap/DoNotRemove) for Infinite
  - `OnOverlap(TargetActor)` / `OnEndOverlap(TargetActor)` blueprint-callable
  - Option: destroy on effect removal

- Blueprint Library (planned)
  - Expose helpers like "Add Source Object to Effect Context"
  - Tag utilities (matches-any-parent checks, etc.)

## Benefits

- Multiplatform: works in SP and MP (with recommended replication modes)
- Enforces separation of concerns (UI MVC)
- Minimizes boilerplate (attributes, delegates, setup)
- Keeps design data in Blueprint, core logic in C++

## Installation (once implemented)

- Copy plugin into Plugins/
- Enable in Project Settings → Plugins
- Set module dependencies if needed
- Swap your game's base classes to plugin bases as desired