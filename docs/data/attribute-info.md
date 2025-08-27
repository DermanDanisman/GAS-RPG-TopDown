# Attribute Info Data Asset

Last updated: 2025-08-27

## Goal

Create a data-driven system for managing attribute metadata using Unreal Engine's Data Asset architecture. The UAttributeInfo Data Asset stores comprehensive information about each attribute, enabling the Widget Controller to provide rich, localized, and designer-friendly attribute displays without hardcoded values in C++ code.

## FAuraAttributeInfo Struct Definition

### Core Structure

The foundational struct that contains all metadata for a single attribute:

```cpp
USTRUCT(BlueprintType)
struct GASCORE_API FAuraAttributeInfo
{
    GENERATED_BODY()

    /** GameplayTag that uniquely identifies this attribute */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    FGameplayTag AttributeTag;

    /** Localized display name for UI elements */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    FText DisplayName;

    /** Localized description for tooltips and detailed information */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    FText Description;

    /** Optional format string for value display (e.g., "{0}%", "{0} pts") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    FText ValueFormat;

    /** Optional icon for UI representation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    TObjectPtr<UTexture2D> AttributeIcon;

    /** Whether this is a primary attribute (affects UI grouping/priority) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    bool bIsPrimary = false;

    /** Default constructor */
    FAuraAttributeInfo()
    {
        AttributeTag = FGameplayTag();
        DisplayName = FText::GetEmpty();
        Description = FText::GetEmpty();
        ValueFormat = FText::FromString("{0}");
        AttributeIcon = nullptr;
        bIsPrimary = false;
    }
};
```

### Field Details

#### AttributeTag (FGameplayTag)
- **Purpose**: Unique identifier linking this info to specific gameplay attributes
- **Usage**: Used by Widget Controller to map attribute changes to display information
- **Examples**: `Attributes.Primary.Strength`, `Attributes.Secondary.CriticalHitChance`

#### DisplayName (FText)
- **Purpose**: Localized, human-readable name for UI display
- **Usage**: Shown in attribute row labels, menus, and tooltips
- **Examples**: "Strength", "Critical Hit Chance", "Armor Penetration"

#### Description (FText)
- **Purpose**: Detailed explanation for tooltips and help text
- **Usage**: Displayed when hovering over attribute rows or in detailed views
- **Examples**: "Increases physical damage and carrying capacity", "Chance to deal double damage on attacks"

#### ValueFormat (FText, Optional)
- **Purpose**: Template for formatting numeric values
- **Usage**: Applied when displaying the attribute value in UI
- **Examples**: `"{0}%"` for percentages, `"{0} pts"` for points, `"{0}"` for raw numbers

#### AttributeIcon (UTexture2D, Optional)
- **Purpose**: Visual representation for the attribute
- **Usage**: Displayed alongside attribute names in UI
- **Examples**: Sword icon for Strength, Brain icon for Intelligence

#### bIsPrimary (bool, Optional)
- **Purpose**: Categorizes attributes for UI organization
- **Usage**: Primary attributes might be displayed prominently, secondary attributes in separate sections
- **Default**: false (treats as secondary attribute)

## UAttributeInfo Data Asset Class

### Class Definition

The Data Asset class that stores and manages collections of attribute information:

```cpp
UCLASS(BlueprintType)
class GASCORE_API UAttributeInfo : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Array of all attribute information entries */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
    TArray<FAuraAttributeInfo> AttributeInformation;

    /** 
     * Find attribute info by GameplayTag
     * @param AttributeTag - The tag to search for
     * @return The corresponding attribute info, or default if not found
     */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    FAuraAttributeInfo FindAttributeInfo(const FGameplayTag& AttributeTag) const;

    /** 
     * Get all primary attributes
     * @return Array of attribute info for primary attributes only
     */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    TArray<FAuraAttributeInfo> GetPrimaryAttributes() const;

    /** 
     * Get all secondary attributes
     * @return Array of attribute info for secondary attributes only
     */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    TArray<FAuraAttributeInfo> GetSecondaryAttributes() const;
};
```

### Implementation Details

#### FindAttributeInfo Function

The core lookup function used by Widget Controllers:

```cpp
FAuraAttributeInfo UAttributeInfo::FindAttributeInfo(const FGameplayTag& AttributeTag) const
{
    for (const FAuraAttributeInfo& Info : AttributeInformation)
    {
        if (Info.AttributeTag.MatchesTagExact(AttributeTag))
        {
            return Info;
        }
    }
    
    // Return default if not found
    return FAuraAttributeInfo();
}
```

#### Helper Functions

Additional utility functions for UI organization:

```cpp
TArray<FAuraAttributeInfo> UAttributeInfo::GetPrimaryAttributes() const
{
    TArray<FAuraAttributeInfo> PrimaryAttributes;
    for (const FAuraAttributeInfo& Info : AttributeInformation)
    {
        if (Info.bIsPrimary)
        {
            PrimaryAttributes.Add(Info);
        }
    }
    return PrimaryAttributes;
}

TArray<FAuraAttributeInfo> UAttributeInfo::GetSecondaryAttributes() const
{
    TArray<FAuraAttributeInfo> SecondaryAttributes;
    for (const FAuraAttributeInfo& Info : AttributeInformation)
    {
        if (!Info.bIsPrimary)
        {
            SecondaryAttributes.Add(Info);
        }
    }
    return SecondaryAttributes;
}
```

## Authoring Guidance

### Creating the Data Asset

1. **Create Blueprint Data Asset**:
   - In Content Browser, right-click → Miscellaneous → Data Asset
   - Choose `UAttributeInfo` as the Data Asset Class
   - Name it `DA_AttributeInfo` or similar

2. **Populate Attribute Information**:
   - Open the Data Asset in the editor
   - Add entries to the `AttributeInformation` array
   - Configure each entry with appropriate values

### Example Entries for Primary Attributes

```cpp
// Strength Attribute
AttributeTag: Attributes.Primary.Strength
DisplayName: "Strength"
Description: "Increases physical damage and carrying capacity. Higher strength allows for more powerful melee attacks and the ability to carry heavier equipment."
ValueFormat: "{0}"
bIsPrimary: true

// Dexterity Attribute  
AttributeTag: Attributes.Primary.Dexterity
DisplayName: "Dexterity"
Description: "Improves accuracy, critical hit chance, and movement speed. Higher dexterity enhances ranged combat effectiveness and evasion abilities."
ValueFormat: "{0}"
bIsPrimary: true

// Intelligence Attribute
AttributeTag: Attributes.Primary.Intelligence
DisplayName: "Intelligence"
Description: "Increases mana capacity and spell effectiveness. Higher intelligence allows for more powerful magic abilities and greater mana reserves."
ValueFormat: "{0}"
bIsPrimary: true
```

### Example Entries for Secondary Attributes

```cpp
// Critical Hit Chance
AttributeTag: Attributes.Secondary.CriticalHitChance
DisplayName: "Critical Hit Chance"
Description: "Probability of dealing double damage on attacks. Calculated based on Dexterity and equipment bonuses."
ValueFormat: "{0}%"
bIsPrimary: false

// Armor Penetration
AttributeTag: Attributes.Secondary.ArmorPenetration
DisplayName: "Armor Penetration"
Description: "Reduces effectiveness of target's armor. Higher penetration allows attacks to bypass more enemy armor."
ValueFormat: "{0}"
bIsPrimary: false

// Mana Regeneration
AttributeTag: Attributes.Secondary.ManaRegeneration
DisplayName: "Mana Regeneration"
Description: "Rate at which mana is restored over time. Based on Intelligence and Willpower attributes."
ValueFormat: "{0}/sec"
bIsPrimary: false
```

### Localization Considerations

- **Use Text Assets**: Store DisplayName and Description in text assets for localization
- **Namespace Properly**: Use consistent namespaces like "AttributeMenu.Labels" and "AttributeMenu.Descriptions"
- **Format Strings**: Ensure ValueFormat templates work across different languages

### Asset Organization

- **Single Source**: Maintain one authoritative Data Asset for all attributes
- **Version Control**: Treat the Data Asset as critical content requiring careful version control
- **Testing**: Create test entries to verify the lookup system works correctly

## Usage in Widget Controller

### Integration Pattern

The Widget Controller uses this Data Asset in step 4 of the data flow:

```cpp
// In UAttributeMenuWidgetController
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
TObjectPtr<UAttributeInfo> AttributeInfoDataAsset;

void UAttributeMenuWidgetController::HandleAttributeChanged(const FGameplayTag& AttributeTag, float NewValue)
{
    if (AttributeInfoDataAsset)
    {
        // Lookup comprehensive attribute info
        FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfo(AttributeTag);
        
        // Update with current value
        AttributeInfo.AttributeValue = NewValue;
        
        // Broadcast to all listening widgets
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
}
```

### Configuration

Controllers need a reference to the Data Asset:

- **Blueprint Configurable**: Expose as BlueprintReadOnly property for designer assignment
- **Default Assignment**: Set default value in constructor or initialization
- **Runtime Flexibility**: Allow swapping Data Assets for different character types or game modes

## Performance Considerations

### Lookup Optimization

- **Linear Search**: Current implementation uses linear search through TArray
- **Small Datasets**: Acceptable for typical attribute counts (10-50 attributes)
- **Optimization Options**: Could use TMap<FGameplayTag, FAuraAttributeInfo> for O(1) lookup if needed

### Memory Usage

- **Lightweight Structs**: FAuraAttributeInfo contains mostly references and simple types
- **Texture Loading**: AttributeIcon textures loaded on-demand by Unreal's asset system
- **Localization**: FText handles localization efficiently without memory duplication

### Caching Strategy

- **Data Asset Persistence**: UAttributeInfo remains in memory once loaded
- **No Runtime Modification**: Attribute information is read-only at runtime
- **Batch Loading**: All attribute info loaded together, avoiding individual asset loads

## Next Steps

1. **Implement Classes**: Create FAuraAttributeInfo struct and UAttributeInfo class in appropriate headers
2. **Create Data Asset**: Author the Blueprint Data Asset with all game attributes
3. **Integrate with Controller**: Update AttributeMenuWidgetController to use the Data Asset
4. **Populate Content**: Add comprehensive attribute information for all primary and secondary attributes
5. **Test Lookup**: Verify FindAttributeInfo correctly retrieves attribute data
6. **Localization Setup**: Prepare text assets for multi-language support

## Related Documentation

- [Attribute Menu Widget Controller](../ui/attribute-menu/attribute-menu-controller.md) - How the Widget Controller uses this Data Asset
- [Gameplay Tags Centralization](../systems/gameplay-tags-centralization.md) - GameplayTag registry that provides the tags used here
- [Attribute Menu Container](../ui/attribute-menu/attribute-menu.md) - The UI that displays this attribute information