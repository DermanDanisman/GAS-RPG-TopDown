# Attribute Menu Widget Controller

Last updated: 2024-12-19

## Goal

Design and implement the AttributeMenuWidgetController that bridges the Gameplay Ability System (GAS) and the Attribute Menu UI, providing a scalable and data-driven approach to attribute display and updates. This controller eliminates the need for individual delegates per attribute by using a generic dispatch pattern with centralized GameplayTag mapping and data asset lookups.

## Prerequisites

- Understanding of the project's Widget Controller pattern (see [UI & Widget Controller](../../ui-widget-controller.md))
- Familiarity with the [Attribute Menu Container](./attribute-menu.md) widget setup
- Basic knowledge of [Attribute Menu Button Row](./text-value-button-row.md) widgets
- Understanding of Unreal's Gameplay Ability System (GAS) and attribute change notifications

## Setup

For detailed setup instructions on creating and configuring the `UAuraAttributeMenuWidgetController`, see the [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) guide.

### Required Overrides Summary

The implementation requires overriding two key methods from `UTDWidgetController`:

- **`BindCallbacksToDependencies()`**: Subscribe to ASC attribute change delegates for all primary and secondary attributes
- **`BroadcastInitialValues()`**: Push initial attribute values to the UI when the controller is first set up

### Construction Pattern Summary  

The controller follows the same caching pattern as other widget controllers:
- Constructed and cached in `AuraHUD` using `WidgetControllerParams`
- Optional Blueprint Function Library helpers for easy widget access
- Data Asset dependency for attribute metadata lookup

See the [setup guide](./attribute-menu-widget-controller-setup.md) for complete implementation details.

## Problem with One-Delegate-Per-Attribute Approach

### Scalability Concerns

The traditional approach of creating individual delegates for each attribute (OnStrengthChanged, OnDexterityChanged, OnIntelligenceChanged, etc.) leads to several issues:

- **Code Explosion**: Each new attribute requires new delegate declarations, binding code, and widget subscriptions
- **Tight Coupling**: Widgets become tightly coupled to specific attribute types
- **Maintenance Overhead**: Adding or removing attributes requires changes in multiple locations
- **Repetitive Patterns**: Similar boilerplate code repeated for each attribute type

### Traditional Pattern Problems

```cpp
// Problematic: Individual delegates for each attribute
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrengthChanged, float, NewStrength);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDexterityChanged, float, NewDexterity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntelligenceChanged, float, NewIntelligence);
// ... dozens more for every attribute
```

## Proposed Pattern: Generic Dispatch with Payload Struct

### FAuraAttributeInfo Structure

The solution uses a single generic delegate that broadcasts a comprehensive data structure containing all necessary information:

```cpp
USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
    GENERATED_BODY()
    
    /** GameplayTag that uniquely identifies this attribute */
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag AttributeTag;
    
    /** Display name for UI (localized) */
    UPROPERTY(BlueprintReadOnly)
    FText DisplayName;
    
    /** Current attribute value */
    UPROPERTY(BlueprintReadOnly)
    float AttributeValue;
    
    /** Description for tooltips/hover text */
    UPROPERTY(BlueprintReadOnly)
    FText Description;
};
```

### Generic Delegate Declaration

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeInfoChanged, const FAuraAttributeInfo&, AttributeInfo);
```

## Data Flow Architecture

### Step-by-Step Flow

#### 1. Ability System Component (ASC) Broadcasts Attribute Change

When an attribute changes in the GAS, the AbilitySystemComponent emits its standard attribute change notifications:

```cpp
// In UCoreAttributeSet or similar
AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(StrengthAttribute)
    .AddLambda([this](const FOnAttributeChangeData& Data) {
        // Attribute changed, now notify controllers
    });
```

#### 2. AttributeMenuWidgetController Subscribes to ASC Delegates

The controller binds to the ASC's attribute change delegates during initialization:

```cpp
void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    // Bind to all relevant attribute change delegates
    const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);
    
    // Bind to primary attributes
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetStrengthAttribute())
        .AddUObject(this, &UAttributeMenuWidgetController::OnStrengthChanged);
        
    // Similar bindings for other attributes...
}
```

#### 3. Controller Maps Changed Attribute to GameplayTag

When an attribute change is detected, the controller maps it to the appropriate GameplayTag using a centralized registry:

```cpp
void UAttributeMenuWidgetController::OnStrengthChanged(const FOnAttributeChangeData& Data)
{
    // Map attribute to tag using centralized registry
    const FGameplayTag AttributeTag = FTDGameplayTags::Get().Attributes_Primary_Strength;
    HandleAttributeChanged(AttributeTag, Data.NewValue);
}
```

#### 4. Controller Queries Attribute Info Data Asset by Tag

The controller looks up comprehensive attribute information from the data asset:

```cpp
void UAttributeMenuWidgetController::HandleAttributeChanged(const FGameplayTag& AttributeTag, float NewValue)
{
    if (AttributeInfoDataAsset)
    {
        FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfo(AttributeTag);
        AttributeInfo.AttributeValue = NewValue;
        
        // Broadcast the complete info to all listening widgets
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
}
```

#### 5. Controller Broadcasts OnAttributeChanged(FAuraAttributeInfo) to Widgets

A single, generic broadcast sends all necessary information to the UI layer:

```cpp
// Single delegate handles all attribute types
FOnAttributeInfoChanged OnAttributeInfoChanged;
```

#### 6. Row Widgets Compare Tags and Update Matching Rows

Each attribute row widget has an AssignedTag and only updates when its tag matches:

```cpp
// In WP_TextValueRow or similar
void UAttributeRowWidget::OnAttributeInfoReceived(const FAuraAttributeInfo& AttributeInfo)
{
    // Only update if this row handles the changed attribute
    if (AssignedTag.MatchesTagExact(AttributeInfo.AttributeTag))
    {
        UpdateLabel(AttributeInfo.DisplayName);
        UpdateValue(FText::AsNumber(AttributeInfo.AttributeValue));
        UpdateTooltip(AttributeInfo.Description);
    }
}
```

## Benefits of This Pattern

### Minimal Coupling

- **Single Delegate**: All attribute rows listen to one dispatcher
- **Tag-Based Filtering**: Rows self-filter based on their assigned GameplayTag
- **Generic Interface**: No widget needs to know about specific attribute types

### Scalability

- **Add New Attributes**: Only requires data asset entries and tag definitions
- **No Code Changes**: Widget and controller code remains unchanged for new attributes
- **Centralized Management**: All attribute metadata managed in one place

### Data-Driven Approach

- **Designer Friendly**: Non-programmers can add/modify attribute information
- **Localization Support**: Display names and descriptions support localization
- **Runtime Flexibility**: Attribute information can be modified without code changes

## Implementation Notes

### Controller Initialization Order

```cpp
// 1. Create controller
UAttributeMenuWidgetController* Controller = NewObject<UAttributeMenuWidgetController>();

// 2. Set widget controller parameters
Controller->SetWidgetControllerParams(WidgetControllerParams);

// 3. Bind controller to widget
AttributeMenuWidget->SetWidgetController(Controller);

// 4. Bind callbacks to dependencies
Controller->BindCallbacksToDependencies();

// 5. Broadcast initial values
Controller->BroadcastInitialValues();
```

### Memory and Performance Considerations

- **Struct Broadcasting**: FAuraAttributeInfo is a lightweight struct suitable for frequent broadcasts
- **Tag Comparison**: GameplayTag comparison is optimized and performant
- **One-to-Many**: Single broadcast updates multiple widgets efficiently

## Integration with Existing Systems

### Compatibility with Widget Controller Pattern

This approach extends the existing CoreWidgetController pattern without breaking changes:

- Inherits from UCoreWidgetController
- Uses established initialization patterns
- Maintains separation of concerns (Model-View-Controller)

### Gameplay Tag Integration

Requires integration with the [centralized GameplayTag registry](../../systems/gameplay-tags-centralization.md) for consistent tag management.

### Data Asset Dependency

Depends on the [UAttributeInfo Data Asset](../../data/attribute-info.md) for attribute metadata storage and retrieval.

## Next Steps

1. **Implement Widget Controller**: Create UAttributeMenuWidgetController inheriting from UCoreWidgetController
2. **Define FAuraAttributeInfo**: Create the payload struct in appropriate header file
3. **Create Data Asset**: Implement UAttributeInfo data asset class and populate with attribute data
4. **Update Row Widgets**: Modify existing attribute row widgets to use tag-based filtering
5. **Centralize Tags**: Implement centralized GameplayTag registry for attribute mapping
6. **Testing**: Verify attribute changes propagate correctly through the new system

## Related Documentation

- [Attribute Menu Container](./attribute-menu.md) - The main UI container this controller manages
- [Text Value Button Row](./text-value-button-row.md) - Row widgets that receive the broadcasts
- [Attribute Info Data Asset](../../data/attribute-info.md) - Data-driven attribute metadata
- [Gameplay Tags Centralization](../../systems/gameplay-tags-centralization.md) - Centralized tag management