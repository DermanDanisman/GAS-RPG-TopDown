# Attribute Info Data Asset

Last updated: 2024-12-19

## Goal

Create a data-driven system for managing attribute metadata using Unreal Engine's Data Asset architecture. The UAttributeInfo Data Asset stores comprehensive information about each attribute, enabling the Widget Controller to provide rich, localized, and designer-friendly attribute displays without hardcoded values in C++ code.

## FAuraAttributeInfo Struct Definition

### Core Structure (From Transcript)

The foundational struct that contains all metadata for a single attribute, based on the transcript requirements:

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
    FText AttributeName;

    /** Localized description for tooltips and detailed information */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    FText AttributeDescription;

    /** Current attribute value (not editable in asset - populated at runtime) */
    UPROPERTY(BlueprintReadOnly, Category = "Attribute Info")
    float AttributeValue = 0.0f;

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
        AttributeName = FText::GetEmpty();
        AttributeDescription = FText::GetEmpty();
        AttributeValue = 0.0f;
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

#### AttributeName (FText)
- **Purpose**: Localized, human-readable name for UI display
- **Usage**: Shown in attribute row labels, menus, and tooltips
- **Examples**: "Strength", "Critical Hit Chance", "Armor Penetration"

#### AttributeDescription (FText)
- **Purpose**: Detailed explanation for tooltips and help text
- **Usage**: Displayed when hovering over attribute rows or in detailed views
- **Examples**: "Increases physical damage and carrying capacity", "Chance to deal double damage on attacks"

#### AttributeValue (float)
- **Purpose**: Current runtime value of the attribute (not editable in asset)
- **Usage**: Populated by the Widget Controller at runtime from the Ability System Component
- **Note**: This field is BlueprintReadOnly and should not be set in the Data Asset editor

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
     * Find attribute info by GameplayTag with optional logging
     * @param AttributeTag - The tag to search for
     * @param bLogNotFound - Whether to log with LogTemp if the tag is not found
     * @return The corresponding attribute info, or default if not found
     */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    FAuraAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

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

#### FindAttributeInfoForTag Function

The core lookup function used by Widget Controllers, with optional logging:

```cpp
FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
    for (const FAuraAttributeInfo& Info : AttributeInformation)
    {
        if (Info.AttributeTag.MatchesTagExact(AttributeTag))
        {
            return Info;
        }
    }
    
    // Log if requested and not found
    if (bLogNotFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttributeInfo not found for tag: %s"), *AttributeTag.ToString());
    }
    
    // Return default if not found
    return FAuraAttributeInfo();
}
```

### Data Asset API

The `UAttributeInfo` data asset provides the following API for iterating through and finding attribute information:

- **FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound=false) const**: Primary lookup function that iterates through the AttributeInformation array and returns the matching entry, optionally logging if not found
- **GetPrimaryAttributes() const**: Returns only primary attributes (bIsPrimary == true) 
- **GetSecondaryAttributes() const**: Returns only secondary attributes (bIsPrimary == false)

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

## Blueprint Authoring Steps

### Step 1: Create DA_AttributeInfo Data Asset

1. **Create Blueprint Data Asset**:
   - In Content Browser, right-click → Miscellaneous → Data Asset
   - Choose `UAttributeInfo` as the Data Asset Class
   - Name it `DA_AttributeInfo`
   - Save in appropriate content folder (e.g., `Content/Data/AttributeInfo/`)

2. **Open Data Asset Editor**:
   - Double-click the created Data Asset
   - Locate the `Attribute Information` array property
   - This array will contain all attribute entries

### Step 2: Populate Attribute Information Entries

For each attribute in your game, add a new entry to the AttributeInformation array:

#### Primary Attribute Example Entries

**Strength Attribute Entry:**
```
AttributeTag: Attributes.Primary.Strength
AttributeName: "Strength"
AttributeDescription: "Increases physical damage and carrying capacity. Higher strength allows for more powerful melee attacks and the ability to carry heavier equipment."
AttributeValue: 0.0 (leave as default - populated at runtime)
ValueFormat: "{0}"
AttributeIcon: [Select appropriate texture]
bIsPrimary: true
```

**Intelligence Attribute Entry:**
```
AttributeTag: Attributes.Primary.Intelligence
AttributeName: "Intelligence"
AttributeDescription: "Increases mana capacity and spell effectiveness. Higher intelligence allows for more powerful magic abilities and greater mana reserves."
AttributeValue: 0.0 (leave as default - populated at runtime)
ValueFormat: "{0}"
AttributeIcon: [Select appropriate texture]
bIsPrimary: true
```

**Resilience Attribute Entry:**
```
AttributeTag: Attributes.Primary.Resilience
AttributeName: "Resilience"
AttributeDescription: "Provides physical and magical resistance. Higher resilience reduces incoming damage from all sources."
AttributeValue: 0.0 (leave as default - populated at runtime)
ValueFormat: "{0}"
AttributeIcon: [Select appropriate texture]
bIsPrimary: true
```

**Vigor Attribute Entry:**
```
AttributeTag: Attributes.Primary.Vigor
AttributeName: "Vigor"
AttributeDescription: "Determines health and stamina capacity. Higher vigor increases maximum health and stamina pools."
AttributeValue: 0.0 (leave as default - populated at runtime)
ValueFormat: "{0}"
AttributeIcon: [Select appropriate texture]
bIsPrimary: true
```

#### Secondary Attribute Example Entries

**Armor Attribute Entry:**
```
AttributeTag: Attributes.Secondary.Armor
AttributeName: "Armor"
AttributeDescription: "Physical damage reduction. Higher armor values reduce incoming physical damage."
AttributeValue: 0.0 (leave as default - populated at runtime)
ValueFormat: "{0}"
AttributeIcon: [Select appropriate texture]
bIsPrimary: false
```

**Critical Hit Chance Entry:**
```
AttributeTag: Attributes.Secondary.CriticalHitChance
AttributeName: "Critical Hit Chance"
AttributeDescription: "Probability of dealing critical damage on attacks. Calculated based on Dexterity and equipment bonuses."
AttributeValue: 0.0 (leave as default - populated at runtime)
ValueFormat: "{0}%"
AttributeIcon: [Select appropriate texture]
bIsPrimary: false
```

### Step 3: Configure Data Asset Properties

1. **GameplayTag Selection**: 
   - Use the GameplayTag picker to select the appropriate tags
   - Ensure tags match those defined in FTDGameplayTags::InitializeNativeGameplayTags()

2. **Text Localization**:
   - For AttributeName and AttributeDescription, consider using String Tables for localization
   - Create entries in a String Table and reference them in the Data Asset

3. **Icon Assignment**:
   - Import appropriate icon textures for each attribute
   - Assign them to the AttributeIcon property for visual representation

### Step 4: Validation and Testing

1. **Tag Consistency**: Verify all AttributeTag values match the native tags in FTDGameplayTags
2. **Completeness**: Ensure all game attributes have corresponding entries
3. **Format Strings**: Test ValueFormat strings render correctly with sample values
4. **Localization**: Verify text displays correctly in different languages if applicable

### Step 5: Integration with Widget Controller

The completed Data Asset will be referenced in the AttributeMenuWidgetController:

```cpp
// In AttributeMenuWidgetController
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
TObjectPtr<UAttributeInfo> AttributeInfoDataAsset;
```

Designers can then assign the DA_AttributeInfo asset to this property in Blueprint or during controller initialization.

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