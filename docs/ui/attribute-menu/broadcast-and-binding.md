# Attribute Menu Broadcast and Binding System

Last updated: 2024-12-19

## What: Delegate-Based Broadcast Pattern

The Attribute Menu system uses a **dynamic multicast delegate** to broadcast `FAuraAttributeInfo` structs from the Widget Controller to multiple subscribing widgets. This creates a reactive, decoupled UI architecture where:

- **Single Source of Truth**: The Widget Controller is the authoritative source for attribute data
- **Multiple Subscribers**: Any number of widgets can bind to receive attribute updates
- **Consistent Data Flow**: All UI elements receive the same structured information simultaneously
- **Decoupled Architecture**: Widgets don't need to know about each other or directly access the AttributeSet

### Core Delegate Declaration

```cpp
// Dynamic multicast delegate for broadcasting attribute information
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeInfoChangedSignature, const FAuraAttributeInfo&, AttributeInfo);

// In Widget Controller class
UPROPERTY(BlueprintAssignable, Category = "GASCore|Attribute Menu")
FOnAttributeInfoChangedSignature OnAttributeInfoChanged;
```

## Why: Benefits of This Architecture

### Scalability and Maintainability
- **No Per-Attribute Delegates**: Instead of creating `OnStrengthChanged`, `OnIntelligenceChanged`, etc., one delegate handles all attributes
- **Easy Attribute Addition**: Adding new attributes requires no controller or widget changes
- **Consistent Broadcasting**: All attributes use the same broadcast pattern and timing

### UI Flexibility
- **Multiple Widget Types**: Text rows, button rows, progress bars can all subscribe to the same data
- **Dynamic Widget Creation**: Widgets can be created at runtime and automatically receive updates
- **Selective Filtering**: Each widget can filter for its specific attribute tag without affecting others

### Data Richness
- **Complete Information**: Each broadcast includes name, description, current value, tag, and formatting
- **Localized Content**: Text fields support localization out of the box
- **Designer-Friendly**: All display information comes from the data asset, not hardcoded strings

## Controller Responsibilities

### 1. Resolve AttributeSet Reference
The controller must maintain a valid reference to the character's AttributeSet to read current values:

```cpp
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    // Ensure we have valid dependencies
    if (!AttributeSet || !AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttributeMenuWidgetController missing dependencies"));
        return;
    }
    
    // Cast to specific AttributeSet implementation
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    // Continue with broadcasting...
}
```

### 2. Look Up Info via UAttributeInfo Data Asset
For each attribute change or initial broadcast, the controller queries the data asset:

```cpp
void UTDAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, float CurrentValue)
{
    if (AttributeInfoDataAsset)
    {
        // Look up comprehensive attribute information from data asset
        FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
        
        // Fill in the runtime value
        AttributeInfo.AttributeValue = CurrentValue;
        
        // Broadcast to all listening widgets
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
}
```

### 3. Populate AttributeValue at Runtime
The `FAuraAttributeInfo.AttributeValue` field is populated from the AttributeSet, not the data asset:

```cpp
// Example: Broadcasting initial Strength value
const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();
BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Primary_Strength, 
                      TDAttributeSet->GetStrength());
```

### 4. Ensure BlueprintAssignable Delegate
The delegate must be marked `BlueprintAssignable` so widgets can bind in Blueprint:

```cpp
/** Blueprint-assignable delegate for attribute info changes */
UPROPERTY(BlueprintAssignable, Category = "GASCore|Attribute Menu|Delegates")
FOnAttributeInfoChangedSignature OnAttributeInfoChanged;
```

### 5. Provide BlueprintCallable BroadcastInitialValues
The base class provides this, but ensure it's accessible from Blueprint:

```cpp
/**
 * Broadcast initial values to the UI.
 * Called once the controller has valid references (PlayerController, PlayerState, ASC, AttributeSet).
 * Override from base to push initial attribute values to widgets.
 */
UFUNCTION(BlueprintCallable, Category = "GASCore|Widget Controller")
virtual void BroadcastInitialValues() override;
```

## Timing: When to Call BroadcastInitialValues

### Critical Timing Requirements

**BroadcastInitialValues must be called AFTER:**
1. `SetWidgetControllerParams()` - Establishes valid ASC/AttributeSet references
2. `Widget.SetWidgetController()` - Widget is ready to receive broadcasts  
3. `BindCallbacksToDependencies()` - Ongoing updates are subscribed
4. **Widget delegate binding** - Most important: widgets must have bound to OnAttributeInfoChanged

### Recommended Call Order

```cpp
// 1. Create and configure controller
UTDAttributeMenuWidgetController* Controller = HUD->GetAttributeMenuWidgetController(WidgetControllerParams);

// 2. Set controller on widget (triggers OnWidgetControllerSet in Blueprint)
AttributeMenuWidget->SetWidgetController(Controller);

// 3. Widget's OnWidgetControllerSet event should:
//    a) Cast controller to correct type
//    b) Bind to OnAttributeInfoChanged delegate
//    c) Set up any other UI bindings

// 4. ONLY AFTER widget binding, call BroadcastInitialValues
Controller->BroadcastInitialValues();
```

### Blueprint Implementation Pattern

In your Attribute Menu widget's Blueprint, implement `OnWidgetControllerSet`:

```
Event OnWidgetControllerSet
├── Cast to TD Attribute Menu Widget Controller
│   └── [Valid] Bind Event to OnAttributeInfoChanged
│       └── Custom Event: UpdateAttributeDisplay
└── [After all bindings complete]
    └── Call BroadcastInitialValues on Controller
```

## Widget Binding Examples

### TextValueButtonRow Binding Pattern

For widgets that display specific attributes (like Strength, Intelligence), implement this pattern:

#### 1. Get Controller Reference
```cpp
// In Widget Blueprint's Event Construct or OnWidgetControllerSet
UTDAttributeMenuWidgetController* AttributeController = 
    UTDBlueprintFunctionLibrary::GetAttributeMenuWidgetController(this);
```

#### 2. Bind to Delegate
```cpp
// Bind to the controller's attribute info delegate
if (AttributeController)
{
    AttributeController->OnAttributeInfoChanged.AddDynamic(this, &UTextValueButtonRow::OnAttributeInfoReceived);
}
```

#### 3. Filter and Update in Callback
```cpp
void UTextValueButtonRow::OnAttributeInfoReceived(const FAuraAttributeInfo& AttributeInfo)
{
    // Check if this update is for our specific attribute
    if (AttributeInfo.AttributeTag.MatchesTagExact(MyAttributeTag))
    {
        // Update UI elements
        AttributeNameText->SetText(AttributeInfo.AttributeName);
        AttributeValueText->SetText(FText::Format(AttributeInfo.ValueFormat, 
                                                  FText::AsNumber(AttributeInfo.AttributeValue)));
        
        // Optional: Update icon if provided
        if (AttributeInfo.AttributeIcon)
        {
            AttributeIcon->SetBrushFromTexture(AttributeInfo.AttributeIcon);
        }
    }
}
```

### Blueprint Binding Example

In Blueprint, the binding looks like:

```
OnWidgetControllerSet Event
├── Cast to TD Attribute Menu Widget Controller
│   └── [Valid] 
│       ├── Bind Event to OnAttributeInfoChanged
│       │   └── Target: Custom Event "Handle Attribute Info"
│       └── Call BroadcastInitialValues
```

```
Custom Event: Handle Attribute Info
├── Input: AttributeInfo (FAuraAttributeInfo)
├── Branch: AttributeInfo.AttributeTag == MyAttributeTag
│   └── [True]
│       ├── Set Text (AttributeNameText) ← AttributeInfo.AttributeName
│       ├── Format Text (AttributeValueText) ← AttributeInfo.ValueFormat + AttributeInfo.AttributeValue
│       └── Set Brush (AttributeIcon) ← AttributeInfo.AttributeIcon
```

## Initial Widget State Handling

### Naive Implementation (Getting Started)
During initial development, you might have all widgets show the same attribute for testing:

```cpp
// Temporary: All rows show Strength for testing
void UTextValueButtonRow::OnAttributeInfoReceived(const FAuraAttributeInfo& AttributeInfo)
{
    const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();
    
    // Naive: Only respond to Strength updates (for testing)
    if (AttributeInfo.AttributeTag.MatchesTagExact(FTDGameplayTags::Get().Attributes_Primary_Strength))
    {
        UpdateUI(AttributeInfo);
    }
}
```

### Production Implementation (Tag-Based Filtering)
In the final implementation, each widget filters by its own attribute tag:

```cpp
// Production: Each widget has its own attribute tag
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute Info")
FGameplayTag MyAttributeTag;

void UTextValueButtonRow::OnAttributeInfoReceived(const FAuraAttributeInfo& AttributeInfo)
{
    // Filter for only our specific attribute
    if (AttributeInfo.AttributeTag.MatchesTagExact(MyAttributeTag))
    {
        UpdateUI(AttributeInfo);
    }
}
```

## Troubleshooting Common Issues

### 1. Widgets Not Updating on Initial Load
**Symptoms**: Widgets remain empty or show default values when first opened
**Causes**: 
- BroadcastInitialValues called before widget binding
- Widget binding failed silently
- Controller reference is null

**Solutions**:
```cpp
// Verify controller reference
if (!AttributeController)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to get AttributeMenuWidgetController"));
    return;
}

// Verify binding succeeded
bool bBindResult = AttributeController->OnAttributeInfoChanged.AddDynamic(this, &UTextValueButtonRow::OnAttributeInfoReceived);
if (!bBindResult)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to bind to OnAttributeInfoChanged"));
}

// Ensure BroadcastInitialValues is called after binding
AttributeController->BroadcastInitialValues();
```

### 2. Null WorldContext in Blueprint Library
**Symptoms**: GetAttributeMenuWidgetController returns null
**Cause**: WorldContext reference is invalid

**Solutions**:
```cpp
// In Blueprint, pass Self (the widget) as WorldContext
UTDAttributeMenuWidgetController* Controller = 
    UTDBlueprintFunctionLibrary::GetAttributeMenuWidgetController(this);

// Verify WorldContext is valid
if (!GetWorld())
{
    UE_LOG(LogTemp, Error, TEXT("Widget has invalid WorldContext"));
    return;
}
```

### 3. Missing Data Asset Reference
**Symptoms**: AttributeInfo broadcasts contain empty/default values
**Cause**: AttributeInfoDataAsset not assigned on controller

**Solutions**:
```cpp
// In controller setup or constructor
void UTDAttributeMenuWidgetController::BeginPlay()
{
    if (!AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset not assigned"));
        
        // Optionally load default asset
        AttributeInfoDataAsset = LoadObject<UAttributeInfo>(nullptr, TEXT("/Game/Data/DA_AttributeInfo"));
    }
}
```

### 4. ASC/AttributeSet Resolution Issues
**Symptoms**: BroadcastInitialValues logs warnings about missing dependencies
**Cause**: AbilitySystemComponent or AttributeSet references are null

**Solutions**:
```cpp
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    // Comprehensive validation
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null"));
        return;
    }
    
    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeSet is null"));
        return;
    }
    
    if (!AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset is null"));
        return;
    }
    
    // Continue with broadcasting...
}
```

### 5. Blueprint Binding Not Working
**Symptoms**: Custom events don't fire when attributes change
**Cause**: Blueprint binding setup incorrect

**Solutions**:
- Ensure the Custom Event has the correct signature: `(FAuraAttributeInfo AttributeInfo)`
- Verify the event is marked as `BlueprintImplementableEvent` or `BlueprintCallable`
- Check that the delegate binding happens in `OnWidgetControllerSet`, not `Event Construct`
- Confirm `BroadcastInitialValues` is called after binding

## Performance Considerations

### Broadcast Frequency
- Attributes typically change infrequently (level ups, equipment changes)
- Frequent updates (health/mana) should use separate delegates in HUD controller
- Consider throttling if attribute changes become too frequent

### Widget Count Impact
- Single broadcast updates all subscribed widgets simultaneously
- O(n) performance where n = number of subscribed widgets
- Widgets should filter quickly using GameplayTag comparison (optimized)

### Memory Usage
- FAuraAttributeInfo is lightweight (mostly references and primitives)
- Broadcast creates temporary copies, but cleanup is automatic
- Consider object pooling if broadcasting becomes very frequent

## Related Documentation

- [Attribute Info Data Asset](../data-asset/attribute-info.md) - Structure and configuration of the data asset
- [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) - Complete controller implementation
- [TextValueButtonRow Widget](./text-value-button-row.md) - Example widget implementation
- [Blueprint Function Library](../blueprint-library/widget-controller-access-guide.md) - Controller access patterns