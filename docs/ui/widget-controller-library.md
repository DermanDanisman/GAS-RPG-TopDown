# Widget Controller Library

## Overview

The **UAuraAbilitySystemLib** Blueprint Function Library provides pure Blueprint nodes to retrieve singleton-like widget controllers from any widget context. This enables widgets to set their own controllers in Event Construct without coupling to specific HUD or overlay implementations.

## Available Nodes

The library exposes two main BlueprintPure functions under the category **UI|Widget Controller**:

### Get HUD Widget Controller

```cpp
UFUNCTION(BlueprintPure, Category = "UI|Widget Controller", 
          meta = (WorldContext = "WorldContext"))
static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContext);
```

**Description**: Retrieves the cached HUD Widget Controller from ATDHUD.

**Parameters**:
- `WorldContext` - Widget or other world context object

**Returns**: The cached HUD widget controller, or `nullptr` if unavailable

### Get Attribute Menu Widget Controller

```cpp
UFUNCTION(BlueprintPure, Category = "UI|Widget Controller",
          meta = (WorldContext = "WorldContext"))
static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);
```

**Description**: Retrieves the cached Attribute Menu Widget Controller from ATDHUD.

**Parameters**:
- `WorldContext` - Widget or other world context object

**Returns**: The cached attribute menu widget controller, or `nullptr` if unavailable

## Usage

### Basic Widget Setup

To use these nodes in your widget's Event Construct:

1. **Event Construct** → **Get Attribute Menu Widget Controller** (or Get HUD Widget Controller)
2. Set the returned controller on your widget
3. Optionally call **Broadcast Initial Values** on the controller

### Blueprint Example

```
Event Construct
├── Get Attribute Menu Widget Controller (WorldContext: Self)
├── Set Widget Controller (Controller: [from previous node])
└── [Optional] Controller → Broadcast Initial Values
```

## How It Works

Both functions follow the same resolution pattern:

1. **Resolve Player Controller**: Uses `UGameplayStatics::GetPlayerController(WorldContext, 0)` to get the local player
2. **Get TDHUD**: Casts the player controller's HUD to `ATDHUD`
3. **Resolve GamePlay Systems**: 
   - Gets PlayerState from PlayerController
   - Resolves AbilitySystemComponent (preferring PlayerState, falling back to Pawn)
   - Resolves AttributeSet by querying `ASC->GetAttributeSet(UTDAttributeSet::StaticClass())`
4. **Build Parameters**: Creates `FGASCoreUIWidgetControllerParams` with all resolved references
5. **Get Controller**: Calls the appropriate TDHUD getter method which handles caching and initialization

## Prerequisites

### BP_TDHUD Configuration

For the library to work correctly, your `BP_TDHUD` must have the following controller classes assigned:

- **HUD Widget Controller Class**: Set to your HUD controller Blueprint or C++ class
- **Attribute Menu Widget Controller Class**: Set to your Attribute Menu controller Blueprint or C++ class

### Required Systems

The library depends on these systems being properly configured:

1. **PlayerState or Pawn**: Must implement `IAbilitySystemInterface` or have an AbilitySystemComponent
2. **AttributeSet**: Must be of type `UTDAttributeSet` (or derived class)
3. **TDHUD**: Must be assigned as the PlayerController's HUD class

## Error Handling

### Common Issues

**Controller is nullptr**: 
- Verify BP_TDHUD controller classes are assigned
- Ensure PlayerController has a valid HUD of type ATDHUD
- Check that AbilitySystemComponent exists on PlayerState or Pawn

**AttributeSet is nullptr**:
- Verify the AbilitySystemComponent has been initialized with UTDAttributeSet
- Check that the AttributeSet was added to the ASC during initialization

**World Context Issues**:
- Ensure you're calling from a valid widget or object with world context
- Use `Self` as WorldContext when calling from widget Event Construct

### Debugging Tips

1. **Check Player Controller**: Verify `GetPlayerController(0)` returns a valid controller
2. **Verify HUD Type**: Ensure the HUD is of type `ATDHUD`, not the base `AHUD`
3. **ASC Availability**: Check that the AbilitySystemComponent is initialized before widget setup
4. **Blueprint Class Assignment**: Verify controller classes are set in BP_TDHUD Details panel

## Benefits

### Decoupling
- Widgets no longer need direct references to HUD or overlay widgets
- Controllers can be accessed from any widget context

### Convenience  
- Single Blueprint node call retrieves fully initialized controllers
- No need to manually resolve PlayerController, PlayerState, ASC, or AttributeSet

### Consistency
- Ensures the same controller instance is reused across all widgets
- Handles initialization automatically on first access

## See Also

- [Attribute Menu Widget Controller Setup](attribute-menu/attribute-menu-widget-controller-setup.md) - Detailed setup guide
- [UI Widget Controller](ui-widget-controller.md) - Base controller patterns
- [TDHUD Documentation](hud/tdhud.md) - HUD implementation details