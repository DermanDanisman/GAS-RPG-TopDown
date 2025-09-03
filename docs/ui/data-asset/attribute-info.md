# Attribute Info Data Asset

Last updated: 2024-12-19

## What: Data Asset Structure for Attribute Metadata

The Attribute Info Data Asset (`UAttributeInfo`) provides a centralized, designer-friendly way to manage all metadata about game attributes. It stores comprehensive information for each attribute including localized names, descriptions, formatting rules, and visual icons, enabling rich UI displays without hardcoding values in C++.

This system separates **static metadata** (stored in the data asset) from **dynamic values** (populated at runtime from the AttributeSet), creating a flexible and maintainable architecture.

## FAuraAttributeInfo Struct: Core Data Structure

### Complete Struct Definition

The `FAuraAttributeInfo` struct contains all information needed to display an attribute in the UI:

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

    /** Current attribute value (populated at runtime, not in data asset) */
    UPROPERTY(BlueprintReadOnly, Category = "Attribute Info")
    float AttributeValue = 0.0f;

    /** Optional format string for value display (e.g., "{0}%", "{0} pts") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    FText ValueFormat;

    /** Optional icon for UI representation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    TObjectPtr<UTexture2D> AttributeIcon;

    /** Whether this is a primary attribute (affects UI grouping) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
    bool bIsPrimary = false;

    /** Default constructor with safe defaults */
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

### Field Descriptions and Usage

#### AttributeTag (FGameplayTag)
- **Purpose**: Unique identifier linking this metadata to actual gameplay attributes
- **Usage**: Used by Widget Controller to map attribute changes to display information  
- **Examples**: `Attributes.Primary.Strength`, `Attributes.Secondary.CriticalHitChance`
- **Requirements**: Must match tags defined in `FTDGameplayTags::InitializeNativeGameplayTags()`

#### AttributeName (FText)
- **Purpose**: Human-readable, localized name for UI display
- **Usage**: Shown in attribute labels, menus, and headers
- **Examples**: "Strength", "Critical Hit Chance", "Armor Penetration"
- **Localization**: Uses FText for automatic localization support

#### AttributeDescription (FText) 
- **Purpose**: Detailed explanation for tooltips and help systems
- **Usage**: Displayed on hover, in detailed views, or help panels
- **Examples**: 
  - "Increases physical damage and carrying capacity"
  - "Chance to deal double damage on attacks" 
  - "Reduces incoming physical damage"

#### AttributeValue (float)
- **Purpose**: Current runtime value of the attribute
- **Important**: This field is **NOT** editable in the data asset - it's populated by the Widget Controller at runtime
- **Usage**: Retrieved from the AttributeSet and filled in during broadcasts
- **Note**: Always starts at 0.0 in the data asset

#### ValueFormat (FText, Optional)
- **Purpose**: Template string for formatting numeric values in UI
- **Usage**: Applied when displaying AttributeValue to users
- **Examples**:
  - `"{0}%"` for percentage values (25%)
  - `"{0} pts"` for point values (15 pts)
  - `"{0}"` for raw numbers (10)
  - `"{0:F1}"` for one decimal place (10.5)

#### AttributeIcon (UTexture2D, Optional)
- **Purpose**: Visual representation for the attribute
- **Usage**: Displayed alongside text in UI elements
- **Examples**: Sword icon for Strength, Brain icon for Intelligence, Shield for Armor
- **Format**: Standard UE texture assets (PNG, JPG, etc.)

#### bIsPrimary (bool, Optional)
- **Purpose**: Categorizes attributes for UI organization and priority
- **Usage**: Primary attributes displayed prominently, secondary in separate sections  
- **Default**: false (treats as secondary attribute)
- **UI Impact**: Affects layout, grouping, and visual emphasis

## UAttributeInfo Data Asset Class

### Class Definition and API

The Data Asset class manages collections of attribute information with lookup functionality:

```cpp
UCLASS(BlueprintType, Blueprintable)
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
     * @param bLogNotFound - Whether to log warning if tag not found
     * @return The corresponding attribute info, or default FAuraAttributeInfo if not found
     */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    FAuraAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

    /** Get all primary attributes (bIsPrimary == true) */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    TArray<FAuraAttributeInfo> GetPrimaryAttributes() const;

    /** Get all secondary attributes (bIsPrimary == false) */
    UFUNCTION(BlueprintCallable, Category = "Attribute Info")
    TArray<FAuraAttributeInfo> GetSecondaryAttributes() const;
};
```

### Core Lookup Function Implementation

The `FindAttributeInfoForTag` function is the primary interface used by Widget Controllers:

```cpp
FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
    // Iterate through all attribute entries
    for (const FAuraAttributeInfo& Info : AttributeInformation)
    {
        if (Info.AttributeTag.MatchesTagExact(AttributeTag))
        {
            return Info;
        }
    }
    
    // Handle not found case
    if (bLogNotFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttributeInfo not found for tag: %s"), *AttributeTag.ToString());
    }
    
    // Return default-constructed struct if not found
    return FAuraAttributeInfo();
}
```

### Helper Functions for UI Organization

Additional utility functions for categorizing attributes:

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

## Configuration: Setting Up the Data Asset

### Step 1: Create the Blueprint Data Asset

1. **Create Data Asset in Content Browser**:
   - Right-click in Content Browser → Miscellaneous → Data Asset
   - Choose `UAttributeInfo` as the Data Asset Class
   - Name it `DA_AttributeInfo` or similar
   - Save in logical location: `Content/Data/Attributes/` or `Content/UI/AttributeInfo/`

2. **Open Data Asset Editor**:
   - Double-click the created asset
   - Locate the `Attribute Information` array property
   - This will contain all your game's attribute entries

### Step 2: Configure Attribute Entries

For each attribute in your game, add an entry to the `AttributeInformation` array:

#### Primary Attribute Examples

**Strength Entry:**
```
Array Element [0]:
├── Attribute Tag: Attributes.Primary.Strength
├── Attribute Name: "Strength"
├── Attribute Description: "Increases physical damage and carrying capacity. Higher strength allows for more powerful melee attacks."
├── Attribute Value: 0.0 (leave default - populated at runtime)
├── Value Format: "{0}"
├── Attribute Icon: [T_Icon_Strength texture]
└── Is Primary: true
```

**Intelligence Entry:**
```
Array Element [1]:
├── Attribute Tag: Attributes.Primary.Intelligence
├── Attribute Name: "Intelligence"
├── Attribute Description: "Increases mana capacity and spell effectiveness. Determines magical power and mana reserves."
├── Attribute Value: 0.0 (leave default)
├── Value Format: "{0}"
├── Attribute Icon: [T_Icon_Intelligence texture]
└── Is Primary: true
```

**Resilience Entry:**
```
Array Element [2]:
├── Attribute Tag: Attributes.Primary.Resilience
├── Attribute Name: "Resilience"
├── Attribute Description: "Provides resistance to physical and magical damage. Increases survivability in combat."
├── Attribute Value: 0.0 (leave default)
├── Value Format: "{0}"
├── Attribute Icon: [T_Icon_Resilience texture]
└── Is Primary: true
```

**Vigor Entry:**
```
Array Element [3]:
├── Attribute Tag: Attributes.Primary.Vigor
├── Attribute Name: "Vigor"
├── Attribute Description: "Determines health and stamina capacity. Higher vigor means more health and stamina."
├── Attribute Value: 0.0 (leave default)
├── Value Format: "{0}"
├── Attribute Icon: [T_Icon_Vigor texture]
└── Is Primary: true
```

#### Secondary Attribute Examples

**Armor Entry:**
```
Array Element [4]:
├── Attribute Tag: Attributes.Secondary.Armor
├── Attribute Name: "Armor"
├── Attribute Description: "Reduces incoming physical damage. Higher values provide better protection against attacks."
├── Attribute Value: 0.0 (leave default)
├── Value Format: "{0}"
├── Attribute Icon: [T_Icon_Armor texture]
└── Is Primary: false
```

**Critical Hit Chance Entry:**
```
Array Element [5]:
├── Attribute Tag: Attributes.Secondary.CriticalHitChance
├── Attribute Name: "Critical Hit Chance"
├── Attribute Description: "Probability of dealing critical damage. Based on dexterity and equipment bonuses."
├── Attribute Value: 0.0 (leave default)
├── Value Format: "{0}%" (shows as percentage)
├── Attribute Icon: [T_Icon_CriticalHit texture]
└── Is Primary: false
```

### Step 3: GameplayTag Configuration

**Critical**: Ensure all `AttributeTag` values exactly match the tags defined in your `FTDGameplayTags` implementation:

```cpp
// In FTDGameplayTags::InitializeNativeGameplayTags()
FTDGameplayTags::Get().Attributes_Primary_Strength = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Attributes.Primary.Strength"), 
                         FString("Primary Attribute: Strength"));
                         
FTDGameplayTags::Get().Attributes_Primary_Intelligence = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Attributes.Primary.Intelligence"), 
                         FString("Primary Attribute: Intelligence"));
// etc...
```

The data asset tags must match these native tag definitions exactly.

### Step 4: Assign Data Asset to Controller

In your Attribute Menu Widget Controller, reference the data asset:

```cpp
// In UAuraAttributeMenuWidgetController or derived class
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
TObjectPtr<UAttributeInfo> AttributeInfoDataAsset;
```

**Configuration Options:**
- **Blueprint Assignment**: Set the property in the controller's Blueprint details
- **Constructor Assignment**: Set default value in C++ constructor
- **Runtime Assignment**: Load and assign programmatically

### Step 5: Controller Integration Example

```cpp
void UAuraAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, float CurrentValue)
{
    if (AttributeInfoDataAsset)
    {
        // Look up metadata from data asset
        FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
        
        // Fill in runtime value from AttributeSet
        AttributeInfo.AttributeValue = CurrentValue;
        
        // Broadcast complete information to UI
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset not assigned to controller"));
    }
}
```

## Forward Declarations and BlueprintType Requirements

### Header File Forward Declarations

To minimize compilation dependencies, use forward declarations where possible:

```cpp
// Forward declarations in header
class UAttributeInfo;
struct FAuraAttributeInfo;
class UTexture2D;

// Full includes in implementation file (.cpp)
#include "Data/AttributeInfo.h"
#include "Engine/Texture2D.h"
```

### BlueprintType and Blueprintable Annotations

For Blueprint compatibility, ensure proper UCLASS and USTRUCT annotations:

```cpp
// Data Asset must be BlueprintType and Blueprintable for editor use
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API UAttributeInfo : public UDataAsset
{
    GENERATED_BODY()
    // ...
};

// Struct must be BlueprintType for Blueprint variable usage
USTRUCT(BlueprintType)
struct GASCORE_API FAuraAttributeInfo
{
    GENERATED_BODY()
    // ...
};

// Properties need BlueprintReadOnly or BlueprintReadWrite for Blueprint access
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
FGameplayTag AttributeTag;
```

### Module Dependency Considerations

If your data asset is in a different module from your UI code:

```cpp
// In your .Build.cs file, ensure proper module dependencies
PublicDependencyModuleNames.AddRange(new string[] 
{
    "Core", 
    "CoreUObject", 
    "Engine",
    "GameplayTags",
    "GASCore",      // If data asset is in GASCore module
    "UMG"           // For UI widgets
});
```

## Usage Patterns in Widget Controllers

### Runtime Value Population Pattern

The most common usage pattern in Widget Controllers:

```cpp
void UAttributeMenuWidgetController::HandleAttributeChanged(const FGameplayTag& AttributeTag, float NewValue)
{
    if (!AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttributeInfoDataAsset is null"));
        return;
    }
    
    // Get static metadata from data asset
    FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
    
    // Populate runtime value
    AttributeInfo.AttributeValue = NewValue;
    
    // Broadcast to UI
    OnAttributeInfoChanged.Broadcast(AttributeInfo);
}
```

### Batch Processing Pattern

For broadcasting multiple attributes at once:

```cpp
void UAttributeMenuWidgetController::BroadcastInitialValues()
{
    if (!AttributeInfoDataAsset || !AttributeSet)
    {
        return;
    }
    
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();
    
    // Broadcast all primary attributes
    TArray<FAuraAttributeInfo> PrimaryAttributes = AttributeInfoDataAsset->GetPrimaryAttributes();
    for (const FAuraAttributeInfo& AttributeInfo : PrimaryAttributes)
    {
        float CurrentValue = GetAttributeValueByTag(AttributeInfo.AttributeTag);
        BroadcastAttributeInfo(AttributeInfo.AttributeTag, CurrentValue);
    }
}
```

### Error Handling Pattern

Robust error handling for production use:

```cpp
FAuraAttributeInfo UAttributeMenuWidgetController::GetSafeAttributeInfo(const FGameplayTag& AttributeTag) const
{
    if (!AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset is null"));
        return FAuraAttributeInfo(); // Return default
    }
    
    if (!AttributeTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid AttributeTag provided"));
        return FAuraAttributeInfo();
    }
    
    FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
    
    // Validate that we found something useful
    if (AttributeInfo.AttributeTag == FGameplayTag())
    {
        UE_LOG(LogTemp, Warning, TEXT("No AttributeInfo found for tag: %s"), *AttributeTag.ToString());
    }
    
    return AttributeInfo;
}
```

## Troubleshooting Common Issues

### 1. Attribute Info Returns Empty/Default Values
**Symptoms**: Widget displays empty names or default descriptions
**Causes**:
- AttributeTag mismatch between data asset and native tags
- Data asset not assigned to controller
- FindAttributeInfoForTag returning default struct

**Solutions**:
```cpp
// Enable logging to see what's happening
FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);

// Validate tag matches
const FTDGameplayTags& Tags = FTDGameplayTags::Get();
if (AttributeTag != Tags.Attributes_Primary_Strength)
{
    UE_LOG(LogTemp, Warning, TEXT("Tag mismatch: Expected %s, got %s"), 
           *Tags.Attributes_Primary_Strength.ToString(), 
           *AttributeTag.ToString());
}
```

### 2. Data Asset Reference Is Null
**Symptoms**: Controller logs "AttributeInfoDataAsset is null"
**Causes**:
- Data asset not assigned in Blueprint
- Data asset failed to load
- Wrong data asset class used

**Solutions**:
```cpp
// Check in controller constructor or BeginPlay
void UAttributeMenuWidgetController::BeginPlay()
{
    if (!AttributeInfoDataAsset)
    {
        // Try to load default asset
        AttributeInfoDataAsset = LoadObject<UAttributeInfo>(nullptr, 
            TEXT("/Game/Data/Attributes/DA_AttributeInfo"));
        
        if (!AttributeInfoDataAsset)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load AttributeInfoDataAsset"));
        }
    }
}
```

### 3. GameplayTag Validation Issues
**Symptoms**: Tags appear valid but lookups fail
**Causes**:
- Tag hierarchy mismatch
- Case sensitivity issues  
- Tag not properly registered

**Solutions**:
```cpp
// Debug tag validation
void UAttributeInfo::ValidateTags() const
{
    const FTDGameplayTags& NativeTags = FTDGameplayTags::Get();
    
    for (const FAuraAttributeInfo& Info : AttributeInformation)
    {
        if (!Info.AttributeTag.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid tag in AttributeInfo array"));
            continue;
        }
        
        // Check if tag exists in native tags
        bool bFoundMatch = false;
        // Compare against all known native tags...
        
        if (!bFoundMatch)
        {
            UE_LOG(LogTemp, Warning, TEXT("Tag %s not found in native tags"), 
                   *Info.AttributeTag.ToString());
        }
    }
}
```

### 4. Blueprint Compilation Issues
**Symptoms**: Blueprint data asset won't compile or save
**Causes**:
- USTRUCT not marked BlueprintType
- Missing BlueprintReadOnly on properties
- Module dependency issues

**Solutions**:
```cpp
// Ensure proper Blueprint annotations
USTRUCT(BlueprintType) // Required for Blueprint usage
struct GASCORE_API FAuraAttributeInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly) // Both required for data assets
    FGameplayTag AttributeTag;
    
    // etc...
};

UCLASS(BlueprintType, Blueprintable) // Both required for data asset creation
class GASCORE_API UAttributeInfo : public UDataAsset
{
    // ...
};
```

## Performance and Memory Considerations

### Lookup Performance
- **Current**: Linear search through TArray (O(n))
- **Acceptable**: For typical attribute counts (5-50 attributes)  
- **Optimization**: Could use TMap<FGameplayTag, FAuraAttributeInfo> for O(1) lookup if needed

### Memory Usage
- **Struct Size**: FAuraAttributeInfo is lightweight (8-16 bytes + text/icon references)
- **Text Localization**: FText handles localization efficiently
- **Icon Loading**: Textures loaded on-demand by UE's asset system

### Loading Strategy
- **Data Asset Loading**: Loaded once at startup, cached in memory
- **Hot Reload Safe**: Changes to data asset are reflected immediately in editor
- **Cooking Optimization**: Asset references resolved at cook time

## Related Documentation

- [Broadcast and Binding System](./broadcast-and-binding.md) - How controllers use this data asset
- [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) - Controller implementation details
- [Gameplay Tags Centralization](../../systems/gameplay-tags-centralization.md) - Native tag definitions
- [TextValueButtonRow Widget](../attribute-menu/text-value-button-row.md) - Example widget consuming this data