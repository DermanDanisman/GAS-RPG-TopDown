# Architecture Overview

Last updated: 2024-12-19

This project follows a clear separation of concerns:

- Model (GAS): ASC, AttributeSet, GameplayEffects, GameplayTags
- Controller (Widget Controller): pulls from Model and broadcasts to View; also receives user inputs from widgets and applies to Model
- View (Widgets): display-only, bind to controller events

Key Classes/Concepts:

- Ability System Component (ASC)
  - Grants and activates abilities
  - Applies and replicates effects
  - Owns tags
  - Useful delegate: OnGameplayEffectAppliedDelegateToSelf

- Attribute Set
  - Stores GameplayAttributeData (Base/Current)
  - Boilerplate accessors macros
  - Use Effects for changes (prefer prediction & rollback)

- Widget Controller
  - Holds references to PlayerController, PlayerState, ASC, AttributeSet
  - Broadcast initial values (health/mana)
  - Subscribes to ASC/AttributeSet delegates and converts to UI signals

- Widgets
  - Call `SetWidgetController(Controller)`
  - Implement `WidgetControllerSet` (BP event) to bind to delegates
  - Use safe percentage calculations (Health / MaxHealth with safe divide)

- Effect Actor Pattern
  - Actor with overlap volume & data for what Effect to apply
  - Applies Instant/Duration/Infinite Effects to overlapping target ASC
  - Policies for Apply/Remove (see ../gas/gameplay-effects.md)