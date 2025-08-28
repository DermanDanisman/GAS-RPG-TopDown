# Attribute Tags and Binding Pattern

Last updated: 2025-01-02

## Overview

This document explains how to assign GameplayTag properties to individual attribute row widgets, establish proper event ordering for initialization, and implement tag-based filtering so each widget updates only when its specific attribute changes. This pattern enables scalable UI design where widgets self-filter based on their assigned AttributeTag rather than requiring per-attribute delegate subscriptions.

## Per-Row AttributeTag Pattern

### Widget Property Setup

Each row widget (TextValueRow, TextValueButtonRow) should have an editable GameplayTag property that identifies which attribute it displays:

```cpp
// In TextValueButtonRow or TextValueRow widget class
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API UTextValueButtonRow : public UUserWidget
{
    GENERATED_BODY()

public:
    /** The specific attribute tag this row displays and filters for */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute Info", 
              meta = (Categories = "Attributes"))
    FGameplayTag AttributeTag;

protected:
    /** Handle attribute info updates from the controller */
    UFUNCTION(BlueprintImplementableEvent, Category = "Attribute Info")
    void OnAttributeInfoReceived(const FAuraAttributeInfo& AttributeInfo);
};
```

### Blueprint Property Configuration

In the Blueprint editor for each row widget:

1. **Select the Widget**: Open your TextValueButtonRow or TextValueRow Blueprint
2. **Locate AttributeTag Property**: Find the "AttributeTag" property in the Details panel under "Attribute Info" category
3. **Toggle "Is Variable"**: Ensure the property is marked as a variable (eye icon should be open)
4. **Set Default Value**: Assign the appropriate GameplayTag for this row:
   - Primary Strength row: `Attributes.Primary.Strength`
   - Primary Intelligence row: `Attributes.Primary.Intelligence`
   - Secondary Armor row: `Attributes.Secondary.Armor`
   - etc.

### Widget Naming Convention

Use descriptive names for row widgets that indicate their purpose:

- **StrengthRow**: Widget displaying Strength attribute
- **IntelligenceRow**: Widget displaying Intelligence attribute
- **ArmorRow**: Widget displaying Armor attribute
- **VigorRow**: Widget displaying Vigor attribute

This makes it easier to identify which widget handles which attribute during setup and debugging.

## Attribute Menu Initialization Sequence

### Event Ordering with Sequence Node

The proper initialization order is critical for ensuring widgets receive initial data. Use a **Sequence** node in the Attribute Menu's Event Construct to enforce the correct timing:

```
Event Construct
    ↓
Sequence Node (Then 0, Then 1, Then 2)
    ├─Then 0─→ Set AttributeTags on Child Widgets
    ├─Then 1─→ Set Widget Controller (triggers child binding)
    └─Then 2─→ Broadcast Initial Values
```

### Step 1: Set AttributeTags on Child Widgets

Before calling any controller methods, assign AttributeTag values to all child row widgets:

```
Custom Function: SetChildAttributeTags
├── Get Widget (StrengthRow) → Set AttributeTag → Attributes.Primary.Strength
├── Get Widget (IntelligenceRow) → Set AttributeTag → Attributes.Primary.Intelligence  
├── Get Widget (ResilienceRow) → Set AttributeTag → Attributes.Primary.Resilience
├── Get Widget (VigorRow) → Set AttributeTag → Attributes.Primary.Vigor
├── Get Widget (ArmorRow) → Set AttributeTag → Attributes.Secondary.Armor
└── Continue for all attribute rows...
```

**Blueprint Implementation:**
1. **Create Custom Function**: Right-click → Add Function → Name: "SetChildAttributeTags"
2. **Add Get Widget Nodes**: For each named child widget
3. **Set AttributeTag Values**: Connect literal GameplayTag values to each widget's AttributeTag property
4. **Call from Sequence**: Connect this function to "Then 0" output of the Sequence node

### Step 2: Set Widget Controller

After AttributeTags are assigned, establish the controller connection:

```
Sequence Then 1:
Get Attribute Menu Widget Controller (World Context: Self)
    ↓
Set Widget Controller
```

This triggers the `OnWidgetControllerSet` event in both the parent menu and all child row widgets, allowing them to bind to the controller's `OnAttributeInfoChanged` delegate.

### Step 3: Broadcast Initial Values

Finally, populate the UI with current attribute data:

```
Sequence Then 2:
[Widget Controller Reference] → Broadcast Initial Values
```

By this point, all widgets have their AttributeTags assigned and have bound to the delegate, ensuring they receive and can filter the initial broadcasts.

## Child Widget Binding and Filtering

### Event Construct in Row Widgets

Each row widget should handle its own delegate binding in Event Construct:

```
Event Construct (in TextValueButtonRow)
    ↓
Branch: Is AttributeTag Valid?
    ├─True─→ Continue with binding setup
    └─False─→ Log Warning: "AttributeTag not set on widget"
```

### OnWidgetControllerSet Implementation

When the parent sets the widget controller, child widgets bind to receive updates:

```
Event OnWidgetControllerSet (in TextValueButtonRow)
    ↓
Cast to TD Attribute Menu Widget Controller
    ├─Success─→ Bind Event to OnAttributeInfoChanged
    │              └─Target: Custom Event "HandleAttributeUpdate"
    └─Failed──→ Log Warning: "Invalid controller type"
```

### Exact Tag Matching Filter

Each widget filters incoming broadcasts for its specific attribute:

```
Custom Event: HandleAttributeUpdate
├── Input: AttributeInfo (FAuraAttributeInfo)
├── Branch: AttributeInfo.AttributeTag.MatchesTagExact(AttributeTag)
│   ├─True─→ Update UI Elements
│   │   ├── Set Text (AttributeNameText) ← AttributeInfo.AttributeName  
│   │   ├── Set Text (AttributeValueText) ← Format: AttributeInfo.AttributeValue
│   │   ├── Set Brush (AttributeIcon) ← AttributeInfo.AttributeIcon
│   │   └── Set Tooltip Text ← AttributeInfo.Description
│   └─False─→ Do Nothing (attribute not for this widget)
```

**Key Points:**
- Use `MatchesTagExact()` not `MatchesTag()` to avoid partial matches
- Only update UI when the exact tag matches
- Perform all UI updates (text, icon, tooltip) in the True branch
- The False branch should remain empty (no logging needed for normal filtering)

## Blueprint Tips and Best Practices

### Using Sequence Nodes for Event Ordering

**Why Use Sequence Nodes:**
- Guarantees execution order of dependent operations
- Makes initialization flow visually clear in Blueprint graphs  
- Prevents race conditions between tag setting and controller binding
- Easy to debug by adding breakpoints to individual sequence outputs

**Sequence Node Setup:**
1. **Right-click in Blueprint**: Add Utility → Flow Control → Sequence
2. **Add Outputs**: Right-click Sequence node → Add pin (up to the number of steps needed)
3. **Connect Input**: Connect Event Construct to Sequence input  
4. **Connect Outputs**: Connect Then 0, Then 1, Then 2 to respective initialization steps

### Widget Reference Management

**Getting Child Widgets:**
- Use **Get Widget** nodes with explicit widget names
- Avoid **Get Children** loops for known, named widgets
- Store frequently-accessed widget references as variables if needed

**Widget Naming Strategy:**
- Use descriptive names: "StrengthRow", "IntelligenceRow", not "Widget_1", "Widget_2"
- Group related widgets with consistent naming: all primary attributes with "Primary" prefix
- Match widget names to their AttributeTag for easier identification

### Debugging and Validation

**AttributeTag Validation:**
```
Custom Function: ValidateAttributeTags
├── For Each Child Widget of Type TextValueButtonRow
│   ├── Get AttributeTag from widget
│   ├── Branch: Is AttributeTag Valid?
│   │   ├─False─→ Print String: "Widget [Name] missing AttributeTag"
│   └── Continue loop
```

**Delegate Binding Validation:**
```
Event OnWidgetControllerSet (in child widget)
├── Bind Event to OnAttributeInfoChanged
├── Branch: Bind Event Result (Success?)
│   ├─True─→ Print String: "Successfully bound to controller"
│   └─False─→ Print String: "Failed to bind to controller"
```

### Comments and Organization

**Blueprint Graph Organization:**
- Group related nodes with **Comment** boxes
- Label comment sections: "1. Set AttributeTags", "2. Controller Setup", "3. Initial Broadcast"
- Use **Reroute** nodes to keep wire connections clean
- Collapse complex logic into **Collapsed Graphs** or **Functions**

**Comment Examples:**
```
Comment Box: "INITIALIZATION SEQUENCE"
"Proper order: Tags → Controller → Broadcast
Ensures widgets can filter initial updates"

Comment Box: "CHILD WIDGET TAG ASSIGNMENT"  
"Set AttributeTag on each row before controller binding
Must happen before OnWidgetControllerSet events fire"
```

## Troubleshooting Common Issues

### Issue 1: Widgets Not Updating

**Symptoms**: Attribute rows remain empty or show default values
**Common Causes**:
- AttributeTag not set or invalid on widget
- AttributeTag doesn't match any broadcast tags exactly
- Widget binding failed or happened too late

**Debugging Steps**:
1. **Verify AttributeTag Values**: Print widget's AttributeTag in Event Construct
2. **Check Tag Matching**: Log both widget AttributeTag and incoming AttributeInfo.AttributeTag  
3. **Validate Binding**: Confirm OnAttributeInfoChanged binding succeeded
4. **Test Broadcast Timing**: Add delays between sequence steps if needed

**Solutions**:
```
Debug Event: HandleAttributeUpdate
├── Print String: "Widget received: " + AttributeInfo.AttributeTag.ToString()
├── Print String: "Widget expects: " + AttributeTag.ToString()  
├── Print String: "Tags match: " + AttributeInfo.AttributeTag.MatchesTagExact(AttributeTag)
```

### Issue 2: Missing "Is Variable" Setting

**Symptoms**: Can't set AttributeTag values in Blueprint, property appears read-only
**Cause**: AttributeTag property not marked as variable in Blueprint

**Solution**:
1. **Select Widget in Blueprint Editor**
2. **Find AttributeTag Property** in Details panel
3. **Click Eye Icon** next to property name to enable "Is Variable"
4. **Compile Blueprint** to apply changes
5. **Set Tag Value** in Default Value field or via Set nodes

### Issue 3: Broadcast Timing Issues

**Symptoms**: Some widgets update, others don't; inconsistent behavior
**Cause**: Race condition between widget binding and controller broadcast

**Solutions**:
- **Use Sequence Nodes**: Enforce proper initialization order
- **Add Small Delays**: Insert 0.1 second delays between sequence steps if needed
- **Validate Controller**: Check controller is valid before broadcasting
- **Implement Retry Logic**: Re-attempt binding if controller not ready

**Retry Pattern Example**:
```
Event Construct
├── Get Attribute Menu Widget Controller
├── Branch: Is Valid?
│   ├─True─→ Continue with normal setup
│   └─False─→ Set Timer: 0.1s, Looping
│               └─Timer Event─→ Retry controller setup
```

### Issue 4: Exact Tag Matching Problems

**Symptoms**: Widget updates for wrong attributes or multiple attributes
**Cause**: Using `MatchesTag()` instead of `MatchesTagExact()`, or tag hierarchy issues

**Solutions**:
- **Use MatchesTagExact()**: Ensures only exact tag matches, not parent tags
- **Verify Tag Strings**: Check for typos in tag names
- **Test Tag Registration**: Ensure tags are properly registered in FAuraGameplayTags
- **Debug Tag Comparison**: Log both tags as strings to compare visually

### Issue 5: WorldContext Resolution Errors

**Symptoms**: "Get Attribute Menu Widget Controller" returns null
**Cause**: WorldContext not properly resolved from widget

**Solutions**:
- **Use Self as WorldContext**: Pass widget's Self reference to library functions
- **Check Player Controller**: Verify player controller exists and is valid
- **Validate HUD Reference**: Ensure ATDHUD is set and accessible
- **Test in PIE vs Standalone**: Some resolution differences between play modes

**WorldContext Debug Pattern**:
```
Get Attribute Menu Widget Controller (World Context: Self)
├── Branch: Is Valid?
│   ├─True─→ Continue with setup
│   └─False─→ Debug WorldContext resolution
│       ├── Get Player Controller (World Context: Self)
│       ├── Print String: "PC Valid: " + Is Valid result  
│       ├── Get HUD from PC
│       └── Print String: "HUD Valid: " + Is Valid result
```

## Multiplayer Considerations

### Client-Server Synchronization

**Attribute Updates**: 
- AttributeSet changes replicate automatically through GAS
- Widget updates happen locally on each client
- No additional replication needed for UI layer

**Controller Access**:
- Each client has its own widget controller instance
- Controllers read from locally-replicated AttributeSet data
- UI updates remain client-side only

### Authority Validation

**Read-Only UI**:
- Attribute display widgets are typically read-only
- No special authority checks needed for display
- Values come from replicated AttributeSet data

**Interactive Elements**:
- Attribute point spending buttons may need authority validation
- Use `HasAuthority()` checks before allowing modifications
- Send attribute changes through proper GAS channels (GameplayEffects)

## Performance Notes

### Delegate Binding Overhead

**Minimal Impact**: 
- Delegate binding is lightweight operation
- Multiple widgets binding to same delegate has negligible performance cost
- Binding happens once during widget initialization

**Memory Usage**:
- Each bound widget maintains a delegate reference
- Automatic cleanup when widgets are destroyed
- No manual unbinding required in most cases

### Tag Comparison Performance

**MatchesTagExact() Cost**:
- Fast string comparison operation
- Optimized for gameplay tag usage patterns
- Much faster than string operations on tag names

**Broadcast Frequency**:
- Initial broadcast happens once per widget creation
- Ongoing broadcasts only on actual attribute changes
- High-frequency attributes (Health/Mana) should use HUD controller instead

## Related Documentation

- [Broadcast and Binding System](./broadcast-and-binding.md) - Overview of the delegate-based broadcast architecture
- [Scalable Broadcasting Plan](./scalable-broadcasting-plan.md) - Strategies for evolving beyond hardcoded per-attribute broadcasts  
- [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) - Controller implementation and initialization
- [Widget Controller Access Guide](../blueprint-library/widget-controller-access-guide.md) - Blueprint Function Library patterns and timing
- [Attribute Info Data Asset](../data-asset/attribute-info.md) - Data asset structure and configuration
- [Attributes Gameplay Tags](../../../systems/attributes-gameplay-tags.md) - GameplayTag definitions and registration