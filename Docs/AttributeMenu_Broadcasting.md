# Attribute Menu Broadcasting (Project Implementation)

This document explains how the Attribute Menu UI is driven by a data asset (`UAttributeInfo`) and how the widget controller (`UTDAttributeMenuWidgetController`) broadcasts both initial values and live updates based on ASC (AbilitySystemComponent) attribute change delegates. This is the implementation currently used in the project.

## Key Concepts

- Data-driven UI metadata is authored in a Data Asset (Tag → Name, Description, and Value).
- The widget controller reads that data to:
  - Broadcast initial values once dependencies are valid.
  - Bind to ASC value change delegates for each attribute to publish updates as values change.
- The current implementation provides a foundation that can be extended with AttributeGetter patterns for more advanced broadcasting.

## Relevant Types (from the project)

- `FTDAttributeInfo` (row in the Data Asset)
- `UAttributeInfo` (Data Asset)
- `UTDAttributeMenuWidgetController` (Widget Controller)

## FTDAttributeInfo and Data Asset Lookup

```cpp
// From AttributeInfo.h
USTRUCT(Blueprintable)
struct FTDAttributeInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag AttributeTag = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText AttributeName = FText();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText AttributeDescription = FText();

    UPROPERTY(BlueprintReadOnly) // set at runtime before broadcast
    float AttributeValue = 0.0f;
};

// UAttributeInfo::FindAttributeInfoForTag (by-value)
FTDAttributeInfo UAttributeInfo::FindAttributeInfoForTag(
    const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
    for (const FTDAttributeInfo& AttributeInfo : AttributeInfos)
    {
        if (AttributeInfo.AttributeTag.MatchesTagExact(AttributeTag))
        {
            return AttributeInfo;
        }
    }

    if (bLogNotFound)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Can't find Attribute Info for AttributeTag [%s] on AttributeInfo [%s]."),
            *AttributeTag.ToString(), *GetNameSafe(this));
    }

    return FTDAttributeInfo();
}
```

## Current Widget Controller Implementation

The current `UTDAttributeMenuWidgetController` provides a basic foundation that inherits from `UGASCoreUIWidgetController`:

```cpp
// From TDAttributeMenuWidgetController.h
UCLASS(BlueprintType, Blueprintable)
class RPG_TOPDOWN_API UTDAttributeMenuWidgetController : public UGASCoreUIWidgetController
{
    GENERATED_BODY()

public:
    virtual void BroadcastInitialValues() override;
    virtual void BindCallbacksToDependencies() override;

    /** Delegate for listening to Health value changes (NewHealth). */
    UPROPERTY(BlueprintAssignable, Category="GASCore|HUD Widget Controller|Attributes")
    FOnAttributeChangedSignature OnHealthChanged;
};

// From TDAttributeMenuWidgetController.cpp
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    Super::BroadcastInitialValues();
    // TODO: Implement attribute broadcasting using AttributeInfo data asset
}

void UTDAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    Super::BindCallbacksToDependencies();
    // TODO: Implement attribute change callback binding
}
```

## Enhanced Implementation Pattern (Intended Design)

Based on the project's architecture and data asset pattern, here's how the complete implementation would work:

```cpp
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    Super::BroadcastInitialValues();
    
    if (!AttributeInfoDataAsset || !AttributeSet || !AbilitySystemComponent)
    {
        return;
    }
    
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    // Iterate through all attribute info entries in the data asset
    for (const auto& AttributeInfoRow : AttributeInfoDataAsset->AttributeInfos)
    {
        if (AttributeInfoRow.AttributeTag.IsValid())
        {
            BroadcastAttributeInfo(AttributeInfoRow.AttributeTag);
        }
    }
}

void UTDAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    Super::BindCallbacksToDependencies();
    
    if (!AttributeInfoDataAsset || !AttributeSet || !AbilitySystemComponent)
    {
        return;
    }
    
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    // Bind to attribute change delegates for each attribute in the data asset
    for (const auto& AttributeInfoRow : AttributeInfoDataAsset->AttributeInfos)
    {
        if (AttributeInfoRow.AttributeTag.IsValid())
        {
            // For each attribute, bind to its change delegate
            // This requires extending the approach to map tags to FGameplayAttribute accessors
            // Example for Health:
            if (AttributeInfoRow.AttributeTag.MatchesTagExact(GameplayTags.Attributes_Vital_Health))
            {
                AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(TDAttributeSet->GetHealthAttribute())
                    .AddLambda([this, AttributeTag = AttributeInfoRow.AttributeTag](const FOnAttributeChangeData& Data)
                    {
                        BroadcastAttributeInfo(AttributeTag);
                    });
            }
            // Similar bindings for other attributes...
        }
    }
}

void UTDAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag) const
{
    if (!AttributeInfoDataAsset)
    {
        return;
    }
    
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    FTDAttributeInfo Info = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag);
    
    // Get current value based on the attribute tag
    if (AttributeTag.MatchesTagExact(GameplayTags.Attributes_Vital_Health))
    {
        Info.AttributeValue = TDAttributeSet->GetHealth();
    }
    else if (AttributeTag.MatchesTagExact(GameplayTags.Attributes_Primary_Strength))
    {
        Info.AttributeValue = TDAttributeSet->GetStrength();
    }
    // Add other attributes as needed...
    
    AttributeInfoDelegate.Broadcast(Info);
}
```

## Attribute Set Architecture

The project uses `UTDAttributeSet` which contains comprehensive attributes:

### Primary Attributes
- Strength, Dexterity, Intelligence, Endurance, Vigor

### Secondary Attributes  
- Armor, ArmorPenetration, BlockChance, CriticalHitChance, CriticalHitDamage, CriticalHitResistance
- HealthRegeneration, ManaRegeneration, StaminaRegeneration
- MaxHealth, MaxMana, MaxStamina

### Vital Attributes
- Health, Mana, Stamina

Each attribute uses the `ATTRIBUTE_ACCESSORS` macro to generate:
- Property getter (`GetStrengthAttribute()`)
- Value getter (`GetStrength()`) 
- Value setter (`SetStrength()`)
- Value initter (`InitStrength()`)

## Event Flow (what happens when an attribute changes?)

1. ASC updates an attribute (e.g., via a Gameplay Effect).
2. ASC broadcasts `FOnAttributeChangeData` for that `FGameplayAttribute`.
3. Our bound lambda fires, calling `BroadcastAttributeInfo(Tag)`.
4. `BroadcastAttributeInfo` looks up metadata (Name/Description), pulls the current numeric value from the AttributeSet, and broadcasts a filled `FTDAttributeInfo` to widgets.

## Why This Approach

- Data asset is the single source of truth for UI-facing metadata.
- Attribute identity can be extended with AttributeGetter patterns (like `FGameplayAttribute` accessors).
- Adding/removing attributes primarily requires updating the Data Asset and extending the controller mapping logic.
- Clear separation between data (AttributeSet), metadata (Data Asset), and presentation logic (Controller).

## Current Limitations and Future Extensions

The current basic implementation can be extended to include:

1. **AttributeGetter Integration**: Add `FGameplayAttribute AttributeGetter` to `FTDAttributeInfo` for generic attribute access.
2. **Generic Broadcasting**: Loop-based broadcasting that doesn't require per-attribute switch statements.
3. **Advanced Binding**: Automatic callback binding using AttributeGetter instead of manual tag-to-accessor mapping.

## Notes and Considerations

- The Data Asset approach provides designer-friendly configuration of attribute metadata.
- `FindAttributeInfoForTag` returns by value, which is convenient for BP but can copy FText. This is acceptable for small sets.
- Ensure `AttributeInfoDataAsset` is assigned in the controller.
- Lifetime: The controller should unbind delegates on teardown if needed (depending on your creation/destruction model).

## Testing Checklist

- Open the Attribute Menu: all rows appear with correct initial values.
- Apply a test GameplayEffect modifying one attribute: only that row updates live.
- Modify a backing attribute for derived stats: dependent rows reflect new values.
- Misconfigured row (missing AttributeTag): logs a warning, does not crash.

## Integration with GASCore

The controller inherits from `UGASCoreUIWidgetController`, which provides:
- Standard initialization pattern via `SetWidgetControllerParams()`
- Base contract for `BroadcastInitialValues()` and `BindCallbacksToDependencies()`
- Integration with the GASCore UI framework

This ensures consistency with the broader GAS UI architecture while allowing project-specific attribute broadcasting logic.