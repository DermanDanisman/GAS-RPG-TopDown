# Attribute Menu Widget Controller (Setup)

Last updated: 2024-12-19

## Goal

Create and configure `UAuraAttributeMenuWidgetController` derived from `UTDWidgetController`, implementing the necessary overrides for `BindCallbacksToDependencies()` and `BroadcastInitialValues()`. This controller bridges the Gameplay Ability System (GAS) and the Attribute Menu UI through a data-driven approach using centralized GameplayTag mapping and Attribute Info Data Assets.

## Prerequisites

- Understanding of the [Widget Controller pattern](../../ui-widget-controller.md)
- Familiarity with [FTDGameplayTags initialization](../../../systems/attributes-gameplay-tags.md)
- Knowledge of [Attribute Info Data Asset structure](../../../data/attribute-info.md)
- Basic GAS attribute change delegate patterns

## UAuraAttributeMenuWidgetController Class Definition

### Header Definition

```cpp
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API UAuraAttributeMenuWidgetController : public UTDWidgetController
{
    GENERATED_BODY()

public:
    /** Broadcast initial attribute values to the UI after setup */
    virtual void BroadcastInitialValues() override;

    /** Subscribe to ASC attribute change delegates */
    virtual void BindCallbacksToDependencies() override;

    /** Generic delegate for broadcasting attribute info changes */
    UPROPERTY(BlueprintAssignable, Category = "Attribute Menu|Delegates")
    FOnAttributeInfoChangedSignature OnAttributeInfoChanged;

protected:
    /** Data Asset containing attribute metadata for lookup */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    TObjectPtr<UAttributeInfo> AttributeInfoDataAsset;

private:
    /** Handle attribute change from GAS and broadcast to UI */
    void HandleAttributeChanged(const FGameplayTag& AttributeTag, float NewValue);
    
    /** Broadcast attribute info for a specific tag */
    void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, float CurrentValue);
};

/** Delegate signature for attribute info changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeInfoChangedSignature, const FAuraAttributeInfo&, AttributeInfo);
```

## Required Method Overrides

### BroadcastInitialValues() Implementation

This method pushes initial attribute values to the UI when the controller is first set up:

```cpp
void UAuraAttributeMenuWidgetController::BroadcastInitialValues()
{
    // Call parent implementation
    Super::BroadcastInitialValues();
    
    // Ensure we have valid dependencies
    if (!AttributeSet || !AttributeInfoDataAsset)
    {
        return;
    }

    const UTDAttributeSet* CoreAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();

    // Broadcast initial values for all primary attributes
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Primary_Strength, 
                          CoreAttributeSet->GetStrength());
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Primary_Intelligence, 
                          CoreAttributeSet->GetIntelligence());
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Primary_Resilience, 
                          CoreAttributeSet->GetResilience());
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Primary_Vigor, 
                          CoreAttributeSet->GetVigor());

    // Broadcast initial values for all secondary attributes
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Secondary_Armor, 
                          CoreAttributeSet->GetArmor());
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Secondary_ArmorPenetration, 
                          CoreAttributeSet->GetArmorPenetration());
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Secondary_BlockChance, 
                          CoreAttributeSet->GetBlockChance());
    BroadcastAttributeInfo(FTDGameplayTags::Get().Attributes_Secondary_CriticalHitChance, 
                          CoreAttributeSet->GetCriticalHitChance());

    // Continue for all attributes...
}
```

### BindCallbacksToDependencies() Implementation

This method subscribes to ASC attribute change delegates:

```cpp
void UAuraAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    // Call parent implementation
    Super::BindCallbacksToDependencies();

    // Ensure we have valid dependencies
    if (!AbilitySystemComponent || !AttributeSet)
    {
        return;
    }

    const UTDAttributeSet* CoreAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();

    // Bind to primary attribute change delegates
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetStrengthAttribute())
        .AddUObject(this, &UAuraAttributeMenuWidgetController::OnStrengthChanged);
        
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetIntelligenceAttribute())
        .AddUObject(this, &UAuraAttributeMenuWidgetController::OnIntelligenceChanged);
        
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetResilienceAttribute())
        .AddUObject(this, &UAuraAttributeMenuWidgetController::OnResilienceChanged);
        
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetVigorAttribute())
        .AddUObject(this, &UAuraAttributeMenuWidgetController::OnVigorChanged);

    // Bind to secondary attribute change delegates
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetArmorAttribute())
        .AddUObject(this, &UAuraAttributeMenuWidgetController::OnArmorChanged);
        
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetArmorPenetrationAttribute())
        .AddUObject(this, &UAuraAttributeMenuWidgetController::OnArmorPenetrationChanged);

    // Continue for all attributes...
}

// Individual attribute change handlers
void UAuraAttributeMenuWidgetController::OnStrengthChanged(const FOnAttributeChangeData& Data)
{
    HandleAttributeChanged(FTDGameplayTags::Get().Attributes_Primary_Strength, Data.NewValue);
}

void UAuraAttributeMenuWidgetController::OnIntelligenceChanged(const FOnAttributeChangeData& Data)
{
    HandleAttributeChanged(FTDGameplayTags::Get().Attributes_Primary_Intelligence, Data.NewValue);
}

// Continue for all attribute change handlers...
```

## Construction Pattern and Caching

### AuraHUD Integration

The controller should be constructed and cached in the AuraHUD, following the same pattern as the overlay controller:

```cpp
// In AuraHUD header
UPROPERTY()
TObjectPtr<UAuraAttributeMenuWidgetController> AttributeMenuWidgetController;

UPROPERTY(EditAnywhere, Category = "Widget Controller")
TSubclassOf<UAuraAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;

// In AuraHUD implementation
UAuraAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
    if (AttributeMenuWidgetController == nullptr)
    {
        AttributeMenuWidgetController = NewObject<UAuraAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
        AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
        AttributeMenuWidgetController->BindCallbacksToDependencies();
    }
    return AttributeMenuWidgetController;
}
```

### WidgetControllerParams Pattern

The controller uses the same initialization parameters as other widget controllers:

```cpp
// WidgetControllerParams contains:
// - PlayerController
// - PlayerState  
// - AbilitySystemComponent
// - AttributeSet

FWidgetControllerParams WCParams;
WCParams.PlayerController = GetOwningPlayerController();
WCParams.PlayerState = GetPlayerState();
WCParams.AbilitySystemComponent = AbilitySystemComponent;
WCParams.AttributeSet = AttributeSet;

// Pass to controller
AttributeMenuWidgetController = AuraHUD->GetAttributeMenuWidgetController(WCParams);
```

## Self-initializing via Blueprint Library

### Overview

Instead of relying on the HUD Overlay to initialize the Attribute Menu widget controller, widgets can now initialize themselves using a Blueprint Function Library. This approach provides better decoupling and allows widgets to manage their own lifecycle.

**Key Benefits**:
- **Decoupling**: Widget doesn't depend on Overlay initialization order
- **Flexibility**: Widget can initialize whenever it's created  
- **Reusability**: Same pattern works for any widget type
- **Singleton Access**: Automatic access to cached controller instances

### Event Construct Workflow

The self-initialization pattern follows this sequence in the widget's **Event Construct**:

```
Event Construct
    ↓
Get Attribute Menu Widget Controller (World Context: Self)
    ↓ [TD Attribute Menu Widget Controller or Null]
Branch (Is Valid?)
    ├── True: Set Widget Controller → (Optional) Broadcast Initial Values
    └── False: Handle Error (retry timer, show loading, log warning)
```

### Detailed Event Construct Steps

1. **Widget Creation**: Attribute Menu widget is created (via CreateWidget or Blueprint spawn)
2. **Event Construct Fires**: UE5 calls Event Construct automatically
3. **Library Function Call**: Widget calls "Get Attribute Menu Widget Controller" with Self as WorldContext
4. **System Resolution**: Library resolves PlayerController → PlayerState → ASC → AttributeSet
5. **HUD Controller Retrieval**: Library calls HUD getter method, returns cached controller
6. **Validation Check**: Widget validates controller is not null using "Is Valid" node
7. **Controller Assignment**: Widget calls "Set Widget Controller" to establish connection
8. **Initial Values**: (Optional) Call "Broadcast Initial Values" to populate UI immediately
9. **UI Ready**: Widget displays current attribute data and receives ongoing updates

### Step-by-Step Blueprint Setup

1. **Open Attribute Menu Widget Blueprint**
   - Navigate to your project's Attribute Menu widget Blueprint
   - Open the Event Graph if not already selected

2. **Locate Event Construct**
   - Find the "Event Construct" node (created automatically)
   - This event fires once when the widget is created

3. **Add Blueprint Library Node**:
   - Right-click in the graph to open context menu
   - Search for "Get Attribute Menu Widget Controller"
   - Add the node to the graph and position after Event Construct

4. **Connect WorldContext**:
   - Connect **Self** pin to **World Context** input on the library node
   - This provides the widget's context for player/world resolution

5. **Add Validation**:
   - Add **Is Valid** node after the library function
   - Connect the controller output to the Is Valid input
   - This prevents errors if the controller isn't available yet

6. **Handle Success Path**:
   - From Is Valid **True** branch, add **Set Widget Controller** node
   - Connect controller reference to Set Widget Controller input
   - This establishes the widget-controller connection

7. **Optional: Add Initial Values**:
   - From controller reference, call **Broadcast Initial Values**
   - This immediately populates the UI with current attribute data
   - Recommended for immediate visual feedback

8. **Handle Failure Path**:
   - From Is Valid **False** branch, add error handling
   - Options: Print String warning, Set Timer for retry, Show placeholder UI
   - Ensures graceful behavior if controller isn't ready

**Example Blueprint Flow**:
```
[Event Construct]
        ↓
[Get Attribute Menu Widget Controller] (World Context: Self)
        ↓
    [Is Valid]
    ├─True──→ [Set Widget Controller] ──→ [Broadcast Initial Values]
    └─False─→ [Print String: "Controller not ready, retrying..."] ──→ [Set Timer: 0.1s, Looping]
```

### Blueprint Library Implementation Example

Create a Blueprint Function Library with TD project naming:

```cpp
UCLASS()
class RPG_TOPDOWN_API UTDWidgetBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Get Attribute Menu Widget Controller from any widget context
     * @param WorldContext - Widget or other world context object
     * @return The cached attribute menu widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "TD Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);

    /**
     * Get HUD Widget Controller from any widget context
     * @param WorldContext - Widget or other world context object  
     * @return The cached HUD widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "TD Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContext);
};

// Implementation
UTDAttributeMenuWidgetController* UTDWidgetBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContext)
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
    {
        if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
        {
            // Resolve all required parameters
            APlayerState* PS = PC->GetPlayerState();
            if (!PS) return nullptr;

            UAbilitySystemComponent* ASC = nullptr;
            if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
            {
                ASC = ASI->GetAbilitySystemComponent();
            }
            if (!ASC) return nullptr;

            UAttributeSet* AttributeSet = ASC->GetAttributeSet(UTDAttributeSet::StaticClass());
            if (!AttributeSet) return nullptr;

            // Compose parameters and get controller from HUD
            const FGASCoreUIWidgetControllerParams WCParams(PC, PS, ASC, AttributeSet);
            return TDHUD->GetAttributeMenuWidgetController(WCParams);
        }
    }
    return nullptr;
}
```

### Timing Considerations

**Important:** Self-initialization may occur before the HUD Overlay is fully initialized:

- **HUD Overlay Timing**: Initialized by `ATDHUD::InitializeHUD()` during game startup
- **Attribute Menu Timing**: Initialized when widget's Event Construct fires
- **Race Condition**: Attribute Menu may construct before player systems are ready

### Defensive Programming

Always check for null returns from Blueprint Library:

```cpp
// In Blueprint: Use "Is Valid" node after Get Attribute Menu Widget Controller
// In C++: Always validate return value

if (UTDAttributeMenuWidgetController* Controller = UTDWidgetBlueprintLibrary::GetAttributeMenuWidgetController(this))
{
    SetWidgetController(Controller);
    Controller->BroadcastInitialValues();
}
else
{
    // Handle case where controller isn't ready yet
    // Consider retry timer or event-driven initialization
}
```

### Benefits of Self-Initialization

- **Decoupling**: Widget doesn't depend on HUD Overlay initialization order
- **Flexibility**: Widget can initialize whenever it's created
- **Reusability**: Same pattern works for any widget type
- **Singleton Access**: Automatic access to cached controller instances

### Implementation Resources

**For Complete Implementation Details**:
- [Attribute Menu Controller Guide](./attribute-menu-controller-guide.md) - Step-by-step HUD setup and Blueprint Library implementation
- [Attribute Menu Node Usage](../blueprint-library/attribute-menu-node-usage.md) - API reference, troubleshooting, and multiplayer considerations  
- [Widget Controller Access Guide](../blueprint-library/widget-controller-access-guide.md) - General Blueprint Function Library patterns

**Quick Start Checklist**:
- [ ] HUD has AttributeMenuWidgetController property and getter method
- [ ] Blueprint Library has GetAttributeMenuWidgetController function  
- [ ] Widget Event Construct calls library function with Self as WorldContext
- [ ] Widget validates controller with "Is Valid" before using
- [ ] Widget calls Set Widget Controller and optionally Broadcast Initial Values

## Blueprint Function Library Helpers (Legacy)

> **Note**: The following section shows legacy Aura naming for reference. Use the self-initializing approach above with proper TD naming instead.

### Utility Functions for Widget Access

Create helper functions to easily retrieve controllers from widgets:

```cpp
UCLASS()
class GASCORE_API UAuraWidgetBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Get Attribute Menu Widget Controller from any widget context
     * @param WorldContext - Widget or other world context object
     * @return The cached attribute menu widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "Aura Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UAuraAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);

    /**
     * Create and initialize Attribute Menu Widget Controller
     * @param WorldContext - Widget or other world context object  
     * @return Newly created and initialized controller
     */
    UFUNCTION(BlueprintCallable, Category = "Aura Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UAuraAttributeMenuWidgetController* CreateAttributeMenuWidgetController(const UObject* WorldContext);
};

// Implementation
UAuraAttributeMenuWidgetController* UAuraWidgetBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContext)
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
    {
        if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
        {
            // Get widget controller params
            FWidgetControllerParams WCParams = AuraHUD->GetWidgetControllerParams();
            return AuraHUD->GetAttributeMenuWidgetController(WCParams);
        }
    }
    return nullptr;
}
```

## Helper Method Implementations

### HandleAttributeChanged Method

```cpp
void UAuraAttributeMenuWidgetController::HandleAttributeChanged(const FGameplayTag& AttributeTag, float NewValue)
{
    BroadcastAttributeInfo(AttributeTag, NewValue);
}
```

### BroadcastAttributeInfo Method

```cpp
void UAuraAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, float CurrentValue)
{
    if (AttributeInfoDataAsset)
    {
        // Look up attribute info from data asset
        FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
        
        // Update with current runtime value
        AttributeInfo.AttributeValue = CurrentValue;
        
        // Broadcast to all listening widgets
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
}
```

## BroadcastInitialValues Timing and Widget Binding

### Critical Timing Requirements

When setting up the Attribute Menu system, the timing of `BroadcastInitialValues()` is crucial. The broadcast must occur **after** widgets have bound to the controller's delegates, or widgets will miss the initial data.

#### Recommended Initialization Sequence

```cpp
// 1. Create and configure controller (usually cached in HUD)
UTDAttributeMenuWidgetController* Controller = HUD->GetAttributeMenuWidgetController(WidgetControllerParams);

// 2. Assign controller to the Attribute Menu widget
AttributeMenuWidget->SetWidgetController(Controller);

// 3. Widget's OnWidgetControllerSet Blueprint event should:
//    - Cast controller to correct type
//    - Bind to OnAttributeInfoChanged delegate  
//    - Set up child widget bindings
//    - Configure any other UI subscriptions

// 4. ONLY AFTER all bindings are complete:
Controller->BroadcastInitialValues();
```

### Widget Binding Pattern

In the Attribute Menu Widget's Blueprint, implement `OnWidgetControllerSet` to handle binding:

#### Blueprint Event Implementation

```
Event OnWidgetControllerSet
├── Cast to TD Attribute Menu Widget Controller
│   ├── [Valid] Store reference as AttributeMenuController
│   └── [Valid] Call BindDelegates Custom Function
└── [After binding] Call BroadcastInitialValues on Controller
```

#### Binding Custom Function Example

```
Custom Function: BindDelegates
├── Input: AttributeMenuController (UTDAttributeMenuWidgetController)
├── Bind Event to OnAttributeInfoChanged
│   └── Target: Custom Event "HandleAttributeInfoUpdate"
├── [Optional] Bind to other controller delegates
└── Return: Success (Boolean)
```

### Child Widget Subscription

Child widgets (TextValueButtonRow, TextValueRow) should subscribe in their own `OnWidgetControllerSet`:

#### TextValueButtonRow Binding Example

```
Event OnWidgetControllerSet (in TextValueButtonRow)
├── Cast to TD Attribute Menu Widget Controller
│   └── [Valid] Bind Event to OnAttributeInfoChanged
│       └── Target: Custom Event "OnAttributeInfoReceived"
```

```
Custom Event: OnAttributeInfoReceived
├── Input: AttributeInfo (FAuraAttributeInfo)
├── Branch: AttributeInfo.AttributeTag == MyAttributeTag
│   └── [True] Update UI Elements
│       ├── Set Text (AttributeNameText) ← AttributeInfo.AttributeName
│       ├── Format Text (AttributeValueText) ← AttributeInfo.ValueFormat + AttributeInfo.AttributeValue
│       └── Set Brush (AttributeIcon) ← AttributeInfo.AttributeIcon
```

### Common Timing Issues and Solutions

#### Issue 1: Empty Widgets on First Load
**Symptoms**: Attribute rows show default/empty values when menu first opens
**Cause**: `BroadcastInitialValues()` called before widget binding

**Solution**: Ensure binding happens before broadcast:
```cpp
// DON'T: Call broadcast immediately after SetWidgetController
AttributeMenuWidget->SetWidgetController(Controller);
Controller->BroadcastInitialValues(); // Too early!

// DO: Let widget handle the timing via OnWidgetControllerSet
AttributeMenuWidget->SetWidgetController(Controller);
// Widget's Blueprint will call BroadcastInitialValues() after binding
```

#### Issue 2: Some Widgets Update, Others Don't
**Symptoms**: Inconsistent widget updates, some show correct data
**Cause**: Race condition between widget binding and broadcast timing

**Solution**: Centralize binding and use proper event order:
```cpp
// In Attribute Menu Widget's OnWidgetControllerSet
void UAttributeMenuWidget::OnWidgetControllerSet_Implementation()
{
    UTDAttributeMenuWidgetController* AttributeController = Cast<UTDAttributeMenuWidgetController>(WidgetController);
    if (!AttributeController)
    {
        return;
    }
    
    // Bind self first
    AttributeController->OnAttributeInfoChanged.AddDynamic(this, &UAttributeMenuWidget::HandleAttributeInfoUpdate);
    
    // Ensure child widgets have bound (may need slight delay or callback system)
    
    // Finally broadcast initial values
    AttributeController->BroadcastInitialValues();
}
```

#### Issue 3: Widgets Binding Too Late
**Symptoms**: Widgets miss initial broadcast, only update on subsequent attribute changes
**Cause**: Child widgets binding after parent broadcasts

**Solution**: Implement cascading binding pattern:
```cpp
// Parent widget coordinates child binding before broadcasting
void UAttributeMenuWidget::EnsureChildrenBound()
{
    // Force child widget initialization if needed
    for (UWidget* Child : GetAllChildren())
    {
        if (UTextValueButtonRow* AttributeRow = Cast<UTextValueButtonRow>(Child))
        {
            AttributeRow->EnsureControllerBound(WidgetController);
        }
    }
}
```

### Blueprint Library Integration

If using Blueprint Function Library helpers, the timing pattern remains the same:

```cpp
// In Widget's OnWidgetControllerSet
Event OnWidgetControllerSet
├── Get Attribute Menu Widget Controller (WorldContext = Self)
├── Store reference for later use
├── Bind to OnAttributeInfoChanged delegate
└── Call BroadcastInitialValues on controller
```

### Troubleshooting Checklist

When widgets aren't showing initial data:

- [ ] **Controller Reference**: Verify `GetAttributeMenuWidgetController()` returns valid controller
- [ ] **Binding Success**: Confirm delegate binding doesn't return false
- [ ] **Delegate Signature**: Ensure Blueprint event signature matches C++ delegate exactly
- [ ] **Broadcast Timing**: Check `BroadcastInitialValues()` called after binding, not before
- [ ] **Widget Lifecycle**: Verify widgets are fully constructed before binding attempts
- [ ] **Controller Dependencies**: Ensure controller has valid ASC/AttributeSet references
- [ ] **Data Asset Assignment**: Confirm `AttributeInfoDataAsset` is assigned and loaded

### Performance Notes

#### Binding Overhead
- Delegate bindings are lightweight once established
- Multiple widgets binding to same delegate has minimal performance impact
- Consider unbinding on widget destruction for memory cleanup

#### Broadcast Frequency  
- Initial broadcast happens once per widget creation
- Ongoing broadcasts only occur on actual attribute changes
- High-frequency attributes (like Health/Mana) should use HUD controller instead

#### Memory Management
- Widget controllers are cached in HUD, not owned by widgets
- Delegate bindings cleaned up automatically when widgets are destroyed
- Data asset remains loaded in memory once referenced

## Configuration and Setup Checklist

### Class Configuration
- [ ] Create UAuraAttributeMenuWidgetController derived from UTDWidgetController
- [ ] Override BroadcastInitialValues() and BindCallbacksToDependencies()
- [ ] Define OnAttributeInfoChanged delegate
- [ ] Add AttributeInfoDataAsset property

### AuraHUD Integration
- [ ] Add AttributeMenuWidgetController property to AuraHUD
- [ ] Add AttributeMenuWidgetControllerClass property for Blueprint configuration
- [ ] Implement GetAttributeMenuWidgetController() method with caching
- [ ] Follow same pattern as existing overlay controller

### Data Asset Configuration
- [ ] Assign DA_AttributeInfo to AttributeInfoDataAsset property
- [ ] Verify all attribute tags in data asset match FTDGameplayTags
- [ ] Test attribute lookup functionality

### Widget Binding Setup
- [ ] Implement OnWidgetControllerSet in Attribute Menu Widget Blueprint
- [ ] Ensure child widgets bind to controller delegates before broadcast
- [ ] Call BroadcastInitialValues() after all binding is complete
- [ ] Test widget updates with debug logging if needed

### Blueprint Function Library (Recommended)
- [ ] Create helper functions for easy controller access from widgets
- [ ] Implement GetAttributeMenuWidgetController() utility using TD naming
- [ ] Enable self-initializing widget pattern for decoupling
- [ ] See [Widget Controller Access Guide](../blueprint-library/widget-controller-access-guide.md) for complete implementation

## Set Row AttributeTags Before Initial Broadcast

### Critical Setup Step

Before calling `BroadcastInitialValues()`, ensure all child row widgets have their `AttributeTag` properties properly assigned. This enables tag-based filtering where each widget only updates when receiving broadcasts for its specific attribute.

### Setup Checklist

- [ ] **Assign AttributeTag Properties**: Set the `AttributeTag` property on each TextValueRow and TextValueButtonRow widget
- [ ] **Toggle "Is Variable"**: Ensure AttributeTag properties are marked as variables in Blueprint editor
- [ ] **Use Descriptive Names**: Name row widgets clearly (StrengthRow, IntelligenceRow, ArmorRow, etc.)
- [ ] **Set Tags Before Controller**: Assign AttributeTags before calling `SetWidgetController` on the parent menu
- [ ] **Validate Tag Values**: Ensure AttributeTag values exactly match tags defined in FTDGameplayTags
- [ ] **Test Tag Matching**: Verify each widget receives only its intended attribute updates

### Implementation Pattern

```
Attribute Menu Event Construct Sequence:
1. Set AttributeTags on all child row widgets
2. Set Widget Controller (triggers child widget binding)
3. Broadcast Initial Values (widgets filter by their AttributeTags)
```

This pattern ensures widgets can properly filter the initial broadcast and display the correct attribute data from the start.

### Cross-References

For complete implementation details, see:
- [Attribute Tags and Binding Pattern](./attribute-tags-and-binding.md) - Step-by-step AttributeTag assignment and filtering implementation
- [Scalable Broadcasting Plan](./scalable-broadcasting-plan.md) - Strategies for evolving beyond hardcoded per-attribute broadcasts

## Related Documentation

- [Attribute Tags and Binding Pattern](./attribute-tags-and-binding.md) - Per-row AttributeTag assignment, widget binding, and exact tag matching implementation
- [Scalable Broadcasting Plan](./scalable-broadcasting-plan.md) - Planning document for evolving from hardcoded broadcasts to data-driven approaches
- [Broadcast and Binding System](./broadcast-and-binding.md) - Detailed guide to the delegate-based broadcast pattern and widget binding
- [Attribute Info Data Asset](../data-asset/attribute-info.md) - Complete data asset structure, configuration, and usage guide
- [Attribute Menu Widget Controller](./attribute-menu-controller.md) - Overall controller design and data flow
- [Attributes Gameplay Tags](../../../systems/attributes-gameplay-tags.md) - Tag initialization and usage patterns
- [UI Widget Controller](../../ui-widget-controller.md) - Base controller patterns and lifecycle
- [Widget Controller Access Guide](../blueprint-library/widget-controller-access-guide.md) - Blueprint Function Library implementation guide
- [Widget Controllers Singletons FAQ](../../faq/widget-controllers-singletons.md) - Common questions and best practices