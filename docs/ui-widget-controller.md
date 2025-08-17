# UI and Widget Controller

Last updated: 2025-08-17

## Pattern

- Widgets do not query the world; they depend on a Widget Controller
- Widget Controller:
  - Knows PlayerController, PlayerState, ASC, AttributeSet
  - Broadcasts initial values and updates via delegates (BlueprintAssignable)
- Widgets:
  - Call `SetWidgetController(Controller)`
  - Handle `WidgetControllerSet` (BlueprintImplementableEvent)
  - Bind to controller delegates for updates

## Example: Health/Mana

- In Overlay Widget Controller:
  - Declare dynamic multicast delegates:
    - `OnHealthChanged(float NewHealth)`
    - `OnMaxHealthChanged(float NewMaxHealth)`
  - Implement `BroadcastInitialValues()`:
    - Cast AttributeSet to your concrete type (e.g., AuraAttributeSet)
    - Broadcast current `GetHealth()` and `GetMaxHealth()`

- In Health Globe Widget (child of a base ProgressBar widget):
  - On `WidgetControllerSet`:
    - Cast controller to Overlay type (or BP subclass)
    - Bind to `OnHealthChanged` and `OnMaxHealthChanged`
    - Cache values, compute percent with safe divide
    - Call `SetProgressBarPercent(percent)`

- HUD init flow:
  - Create Overlay Widget and its Controller
  - Set Overlay's Widget Controller (this triggers `WidgetControllerSet`)
  - After binding, tell the controller to `BroadcastInitialValues()`