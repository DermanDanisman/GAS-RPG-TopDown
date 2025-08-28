# Attribute Menu Widget Controller (Setup)

Last updated: 2025-01-26

## Goal

Create and configure `UAuraAttributeMenuWidgetController` derived from `UAuraWidgetController`, implementing the necessary overrides for `BindCallbacksToDependencies()` and `BroadcastInitialValues()`. This controller bridges the Gameplay Ability System (GAS) and the Attribute Menu UI through a data-driven approach using centralized GameplayTag mapping and Attribute Info Data Assets.

## Prerequisites

- Understanding of the [Widget Controller pattern](../../ui-widget-controller.md)
- Familiarity with [FAuraGameplayTags initialization](../../../systems/attributes-gameplay-tags.md)
- Knowledge of [Attribute Info Data Asset structure](../../../data/attribute-info.md)
- Basic GAS attribute change delegate patterns

## UAuraAttributeMenuWidgetController Class Definition

### Header Definition

```cpp
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API UAuraAttributeMenuWidgetController : public UAuraWidgetController
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

    const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

    // Broadcast initial values for all primary attributes
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Strength, 
                          CoreAttributeSet->GetStrength());
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Intelligence, 
                          CoreAttributeSet->GetIntelligence());
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Resilience, 
                          CoreAttributeSet->GetResilience());
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Vigor, 
                          CoreAttributeSet->GetVigor());

    // Broadcast initial values for all secondary attributes
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_Armor, 
                          CoreAttributeSet->GetArmor());
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_ArmorPenetration, 
                          CoreAttributeSet->GetArmorPenetration());
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_BlockChance, 
                          CoreAttributeSet->GetBlockChance());
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_CriticalHitChance, 
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

    const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

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
    HandleAttributeChanged(FAuraGameplayTags::Get().Attributes_Primary_Strength, Data.NewValue);
}

void UAuraAttributeMenuWidgetController::OnIntelligenceChanged(const FOnAttributeChangeData& Data)
{
    HandleAttributeChanged(FAuraGameplayTags::Get().Attributes_Primary_Intelligence, Data.NewValue);
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

## Blueprint Function Library Helpers

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

## Configuration and Setup Checklist

### Class Configuration
- [ ] Create UAuraAttributeMenuWidgetController derived from UAuraWidgetController
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
- [ ] Verify all attribute tags in data asset match FAuraGameplayTags
- [ ] Test attribute lookup functionality

### Blueprint Function Library (Optional)
- [ ] Create helper functions for easy controller access from widgets
- [ ] Implement GetAttributeMenuWidgetController() utility
- [ ] Add CreateAttributeMenuWidgetController() for initialization

## Self-initializing via Blueprint Library

The new **UAuraAbilitySystemLib** Blueprint Function Library allows Attribute Menu widgets to initialize themselves without coupling to overlay or HUD implementations.

### Setup Steps

1. **Event Construct**: In your Attribute Menu widget's Event Construct
2. **Get Controller**: Add a **Get Attribute Menu Widget Controller** node (found under UI|Widget Controller category)
   - Set **World Context** to **Self**
3. **Set Controller**: Connect the returned controller to your widget's **Set Widget Controller** method
4. **Optional Broadcast**: Call **Broadcast Initial Values** on the controller if needed

### Blueprint Node Setup

```
Event Construct
├── Get Attribute Menu Widget Controller (World Context: Self)
├── Set Widget Controller (Controller: [output from previous node])
└── [Optional] Controller → Broadcast Initial Values
```

### Benefits

- **No HUD Coupling**: Widget doesn't need references to specific HUD or overlay widgets  
- **Automatic Resolution**: Library automatically resolves PlayerController, PlayerState, ASC, and AttributeSet
- **Controller Caching**: Returns the same cached controller instance across multiple widgets
- **Error Handling**: Returns `nullptr` if prerequisites aren't met, allowing graceful failure

### Prerequisites

Ensure your **BP_TDHUD** has the **Attribute Menu Widget Controller Class** property set to your controller Blueprint class.

### Troubleshooting

**Controller is null**: 
- Verify BP_TDHUD has AttributeMenuWidgetControllerClass assigned
- Check that PlayerController has valid ASC and AttributeSet
- Ensure the widget has proper world context

For complete library documentation, see [Widget Controller Library](../widget-controller-library.md).

## Related Documentation

- [Attribute Menu Widget Controller](./attribute-menu-controller.md) - Overall controller design and data flow
- [Attributes Gameplay Tags](../../../systems/attributes-gameplay-tags.md) - Tag initialization and usage patterns
- [Attribute Info Data Asset](../../../data/attribute-info.md) - Data asset structure and authoring
- [UI Widget Controller](../../ui-widget-controller.md) - Base controller patterns and lifecycle