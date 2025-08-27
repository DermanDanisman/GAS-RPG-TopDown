# Gameplay Tags Centralization

Last updated: 2025-08-27

## Goal

Establish a centralized system for managing GameplayTags throughout the project to avoid scattered `RequestGameplayTag` calls with raw FNames, improve compile-time safety, and ensure consistent tag usage across C++ code and Blueprint systems. This centralization is essential for the Attribute Menu data flow where tags serve as the bridge between attribute changes and UI updates.

## Problem with Scattered Tag Requests

### Issues with Ad-Hoc Tag Creation

Using `RequestGameplayTag` with string literals throughout the codebase leads to several problems:

```cpp
// Problematic: Scattered throughout different files
FGameplayTag StrengthTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Attributes.Primary.Strength"));
FGameplayTag DexTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Attributes.Primary.Dexterity"));
```

**Problems:**
- **Typo-Prone**: String literals are vulnerable to spelling errors
- **No Compile-Time Safety**: Errors only discovered at runtime
- **Refactoring Difficulty**: Changing tag names requires finding all string occurrences
- **No IDE Support**: No auto-completion or go-to-definition for tag references
- **Maintenance Overhead**: Difficult to track which tags are used where

### Blueprint Integration Issues

- **String Duplication**: Same tag strings repeated in both C++ and Blueprints
- **Synchronization Problems**: C++ and Blueprint tag references can diverge
- **No Central Registry**: No single source of truth for available tags

## Centralized GameplayTag Management Options

### Option 1: Native Gameplay Tags Registration (Recommended)

The preferred approach for compile-time safety and performance:

#### GameplayTags.ini Configuration

Create or update `Config/DefaultGameplayTags.ini`:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
ImportTagsFromConfig=True
WarnOnInvalidTags=True
ClearInvalidTags=False
AllowEditorTagUnloading=True
AllowGameTagUnloading=False
FastReplication=False
InvalidTagCharacters="\"\',"
NumBitsForContainerSize=6
NetIndexFirstBitSegment=16

[/Script/GameplayTags.GameplayTagsList]
GameplayTagList=(Tag="Attributes",DevComment="Root tag for all attribute-related tags")
GameplayTagList=(Tag="Attributes.Primary",DevComment="Primary character attributes")
GameplayTagList=(Tag="Attributes.Primary.Strength",DevComment="Physical power attribute")
GameplayTagList=(Tag="Attributes.Primary.Dexterity",DevComment="Agility and precision attribute")
GameplayTagList=(Tag="Attributes.Primary.Intelligence",DevComment="Mental acuity and mana capacity")
GameplayTagList=(Tag="Attributes.Primary.Willpower",DevComment="Mental fortitude and mana regeneration")
GameplayTagList=(Tag="Attributes.Secondary",DevComment="Secondary derived attributes")
GameplayTagList=(Tag="Attributes.Secondary.Armor",DevComment="Physical damage reduction")
GameplayTagList=(Tag="Attributes.Secondary.ArmorPenetration",DevComment="Armor bypass capability")
GameplayTagList=(Tag="Attributes.Secondary.BlockChance",DevComment="Probability of blocking attacks")
GameplayTagList=(Tag="Attributes.Secondary.CriticalHitChance",DevComment="Probability of critical strikes")
GameplayTagList=(Tag="Attributes.Secondary.CriticalHitDamage",DevComment="Critical strike damage multiplier")
GameplayTagList=(Tag="Attributes.Secondary.CriticalHitResistance",DevComment="Resistance to critical strikes")
GameplayTagList=(Tag="Attributes.Secondary.HealthRegeneration",DevComment="Health restoration rate")
GameplayTagList=(Tag="Attributes.Secondary.ManaRegeneration",DevComment="Mana restoration rate")
GameplayTagList=(Tag="Attributes.Secondary.StaminaRegeneration",DevComment="Stamina restoration rate")
GameplayTagList=(Tag="Attributes.Secondary.MaxHealth",DevComment="Maximum health capacity")
GameplayTagList=(Tag="Attributes.Secondary.MaxMana",DevComment="Maximum mana capacity")
GameplayTagList=(Tag="Attributes.Secondary.MaxStamina",DevComment="Maximum stamina capacity")
```

#### Benefits of Native Registration

- **Compile-Time Validation**: Tags validated during compilation
- **Performance**: Native tags are faster than RequestGameplayTag
- **Editor Integration**: Tags appear in dropdown menus and property panels
- **Blueprint Accessibility**: Automatically available in Blueprint editors
- **Version Control**: Tag definitions stored in config files, tracked by source control

### Option 2: Central FAuraGameplayTags Struct/Class (Alternative)

A C++ struct that centralizes tag creation and provides typed access:

#### Header Definition (FAuraGameplayTags.h)

```cpp
USTRUCT(BlueprintType)
struct GASCORE_API FAuraGameplayTags
{
    GENERATED_BODY()

    // Singleton access
    static const FAuraGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeGameplayTags();

    // Primary Attributes
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Primary")
    FGameplayTag Attributes_Primary_Strength;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Primary")
    FGameplayTag Attributes_Primary_Dexterity;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Primary")
    FGameplayTag Attributes_Primary_Intelligence;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Primary")
    FGameplayTag Attributes_Primary_Willpower;

    // Secondary Attributes
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary")
    FGameplayTag Attributes_Secondary_Armor;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary")
    FGameplayTag Attributes_Secondary_ArmorPenetration;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary")
    FGameplayTag Attributes_Secondary_BlockChance;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary")
    FGameplayTag Attributes_Secondary_CriticalHitChance;

    // Add more as needed...

private:
    static FAuraGameplayTags GameplayTags;
};
```

#### Implementation (FAuraGameplayTags.cpp)

```cpp
FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // Primary Attributes
    GameplayTags.Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Strength"),
        FString("Physical power attribute")
    );

    GameplayTags.Attributes_Primary_Dexterity = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Dexterity"),
        FString("Agility and precision attribute")
    );

    GameplayTags.Attributes_Primary_Intelligence = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Intelligence"),
        FString("Mental acuity and mana capacity")
    );

    // Secondary Attributes
    GameplayTags.Attributes_Secondary_Armor = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.Armor"),
        FString("Physical damage reduction")
    );

    GameplayTags.Attributes_Secondary_CriticalHitChance = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.CriticalHitChance"),
        FString("Probability of critical strikes")
    );

    // Continue for all attributes...
}
```

#### Initialization Call

In your main game module or startup code:

```cpp
void FAuraModule::StartupModule()
{
    // Initialize tags on module startup
    FAuraGameplayTags::InitializeNativeGameplayTags();
}
```

### Recommended Hybrid Approach

**Best Practice**: Combine both approaches:

1. **Define in Config**: Use GameplayTags.ini for tag hierarchy and descriptions
2. **Native Access**: Create FAuraGameplayTags for compile-time safety and performance
3. **Initialization**: Load config-defined tags into native struct on startup

## Usage in Attribute Menu Flow

### Mapping Attributes to Tags

The centralized tag system enables clean attribute-to-tag mapping in the Widget Controller:

```cpp
void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);
    
    // Clean, compile-time safe tag references
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetStrengthAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
            HandleAttributeChanged(FAuraGameplayTags::Get().Attributes_Primary_Strength, Data.NewValue);
        });
        
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetDexterityAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
            HandleAttributeChanged(FAuraGameplayTags::Get().Attributes_Primary_Dexterity, Data.NewValue);
        });
        
    // Continue for all attributes...
}
```

### Tag-Based Attribute Info Lookup

The centralized tags work seamlessly with the Data Asset lookup:

```cpp
void UAttributeMenuWidgetController::HandleAttributeChanged(const FGameplayTag& AttributeTag, float NewValue)
{
    if (AttributeInfoDataAsset)
    {
        // Tag comes from centralized registry, Data Asset uses same tag for lookup
        FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfo(AttributeTag);
        AttributeInfo.AttributeValue = NewValue;
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
}
```

### Blueprint Integration

Centralized tags are automatically available in Blueprints:

```cpp
// Blueprint nodes can directly reference centralized tags
// - Get Aura Gameplay Tags -> Attributes Primary Strength
// - Tag comparison nodes work with centralized tags
// - Data Asset references use same centralized tag values
```

## Tag Naming Conventions

### Hierarchical Structure

```
Attributes
├── Primary
│   ├── Strength
│   ├── Dexterity
│   ├── Intelligence
│   └── Willpower
└── Secondary
    ├── Armor
    ├── ArmorPenetration
    ├── BlockChance
    ├── CriticalHitChance
    ├── CriticalHitDamage
    ├── CriticalHitResistance
    ├── HealthRegeneration
    ├── ManaRegeneration
    ├── StaminaRegeneration
    ├── MaxHealth
    ├── MaxMana
    └── MaxStamina
```

### Naming Rules

- **Use PascalCase**: Each word capitalized (e.g., "CriticalHitChance")
- **Dot Separation**: Hierarchical levels separated by dots
- **Descriptive Names**: Full words rather than abbreviations
- **Consistent Prefixes**: All attributes start with "Attributes"
- **Logical Grouping**: Group related concepts (Primary vs Secondary)

## Integration with Existing Systems

### Compatibility with GAS

- **Native Integration**: GameplayTags are core GAS components
- **Effect Integration**: Tags can be used for Gameplay Effect Asset Tags
- **Ability Gating**: Same tags can gate abilities based on attribute requirements
- **Condition Checking**: Tag-based attribute conditions for skills/abilities

### Performance Considerations

- **Tag Comparison**: GameplayTag comparison is highly optimized
- **Memory Usage**: Tags are stored as FNames internally, memory efficient
- **Network Replication**: Tags replicate efficiently in multiplayer scenarios
- **Lookup Speed**: Native tags provide O(1) access, RequestGameplayTag is slower

## Migration Strategy

### From Scattered Strings to Centralized Tags

1. **Audit Current Usage**: Find all RequestGameplayTag and string literal usage
2. **Define Tag Hierarchy**: Create comprehensive tag structure in config
3. **Create Native Struct**: Implement FAuraGameplayTags for C++ access
4. **Replace String Literals**: Convert hardcoded strings to centralized references
5. **Update Blueprints**: Replace string-based tag nodes with native references
6. **Test Integration**: Verify all systems work with centralized tags

### Incremental Approach

- **Start with Attributes**: Focus on attribute-related tags first
- **Add Systems Gradually**: Extend to abilities, effects, and UI as needed
- **Maintain Compatibility**: Keep old string-based code working during transition
- **Document Changes**: Update all relevant documentation with new tag usage

## Best Practices

### Development Workflow

- **Design First**: Plan tag hierarchy before implementation
- **Review Process**: Have tag additions reviewed by team
- **Documentation**: Document the purpose of each tag
- **Testing**: Test tag functionality in both C++ and Blueprints

### Maintenance Guidelines

- **Single Source**: All tag definitions in one location (config or native)
- **Version Control**: Track tag changes in source control
- **Backwards Compatibility**: Consider impact of tag name changes
- **Cleanup**: Remove unused tags periodically

## Next Steps

1. **Choose Approach**: Decide between native config, C++ struct, or hybrid approach
2. **Define Tag Hierarchy**: Create comprehensive attribute tag structure
3. **Implement Centralization**: Create config files or native struct as chosen
4. **Update Widget Controller**: Modify attribute mapping to use centralized tags
5. **Integrate with Data Asset**: Ensure Data Asset uses same centralized tags
6. **Test End-to-End**: Verify complete attribute change → UI update flow
7. **Expand Usage**: Apply centralized tags to other game systems

## Related Documentation

- [Attribute Menu Widget Controller](../ui/attribute-menu/attribute-menu-controller.md) - How the Widget Controller uses centralized tags for attribute mapping
- [Attribute Info Data Asset](../data/attribute-info.md) - Data Asset that uses these tags as lookup keys
- [Gameplay Tags](../gas/gameplay-tags.md) - General GameplayTag usage in the project