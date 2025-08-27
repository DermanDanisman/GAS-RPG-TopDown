# Attributes Gameplay Tags (Primary and Secondary)

Last updated: 2025-01-26

## Goal

Document the pattern for registering primary and secondary attribute tags natively in `FAuraGameplayTags::InitializeNativeGameplayTags()`, which is called by `UAuraAssetManager::StartInitialLoading()`. This provides centralized access to attribute tags through the singleton pattern, ensuring compile-time safety and consistent tag management across the attribute system.

## Native Tag Registration Pattern

### Initialization Flow

The attribute tags are registered during game startup through this call chain:

1. `UAuraAssetManager::StartInitialLoading()` - Called during asset manager initialization
2. `FAuraGameplayTags::InitializeNativeGameplayTags()` - Registers all native tags
3. `UGameplayTagsManager::AddNativeGameplayTag()` - Creates and returns each tag

### Implementation Example

```cpp
// In FAuraGameplayTags::InitializeNativeGameplayTags()
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // Primary Attributes - assign returns to FAuraGameplayTags members
    GameplayTags.Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Strength"),
        FString("Physical power and melee damage")
    );

    GameplayTags.Attributes_Primary_Intelligence = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Intelligence"),
        FString("Mental acuity and magical power")
    );

    GameplayTags.Attributes_Primary_Resilience = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Resilience"),
        FString("Physical and magical resistance")
    );

    GameplayTags.Attributes_Primary_Vigor = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Vigor"),
        FString("Health and stamina capacity")
    );

    // Secondary Attributes - derived from primary attributes
    GameplayTags.Attributes_Secondary_Armor = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.Armor"),
        FString("Physical damage reduction")
    );

    GameplayTags.Attributes_Secondary_ArmorPenetration = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.ArmorPenetration"),
        FString("Bypasses enemy armor")
    );

    GameplayTags.Attributes_Secondary_BlockChance = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.BlockChance"),
        FString("Probability of blocking incoming attacks")
    );

    GameplayTags.Attributes_Secondary_CriticalHitChance = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.CriticalHitChance"),
        FString("Probability of dealing critical damage")
    );

    // Continue for all secondary attributes...
}
```

## Naming Conventions

### Hierarchical Structure

All attribute tags follow a consistent hierarchical naming pattern:

- **Primary Attributes**: `Attributes.Primary.<AttributeName>`
- **Secondary Attributes**: `Attributes.Secondary.<AttributeName>`

### FAuraGameplayTags Member Names

The singleton members use underscore-separated naming to match the tag hierarchy:

```cpp
// Primary attributes
FGameplayTag Attributes_Primary_Strength;
FGameplayTag Attributes_Primary_Intelligence;
FGameplayTag Attributes_Primary_Resilience;
FGameplayTag Attributes_Primary_Vigor;

// Secondary attributes  
FGameplayTag Attributes_Secondary_Armor;
FGameplayTag Attributes_Secondary_ArmorPenetration;
FGameplayTag Attributes_Secondary_BlockChance;
FGameplayTag Attributes_Secondary_CriticalHitChance;
```

## AddNativeGameplayTag Pattern

### Pattern Details

The `AddNativeGameplayTag` method follows this signature:

```cpp
FGameplayTag UGameplayTagsManager::AddNativeGameplayTag(
    FName TagName,
    const FString& TagDevComment = FString()
)
```

### Key Benefits

- **Return Value Assignment**: The method returns the created tag, which is immediately assigned to the singleton member
- **Runtime Descriptions**: DevComment provides searchable descriptions in the editor
- **Centralized Registration**: All tags registered in one initialization function
- **Startup Order**: Called early in the loading process via AssetManager

## Example Tags from Transcript

### Primary Attributes

| Tag Name | Singleton Member | Description |
|----------|------------------|-------------|
| `Attributes.Primary.Strength` | `Attributes_Primary_Strength` | Physical power and melee damage |
| `Attributes.Primary.Intelligence` | `Attributes_Primary_Intelligence` | Mental acuity and magical power |
| `Attributes.Primary.Resilience` | `Attributes_Primary_Resilience` | Physical and magical resistance |
| `Attributes.Primary.Vigor` | `Attributes_Primary_Vigor` | Health and stamina capacity |

### Secondary Attributes

| Tag Name | Singleton Member | Description |
|----------|------------------|-------------|
| `Attributes.Secondary.Armor` | `Attributes_Secondary_Armor` | Physical damage reduction |
| `Attributes.Secondary.ArmorPenetration` | `Attributes_Secondary_ArmorPenetration` | Bypasses enemy armor |
| `Attributes.Secondary.BlockChance` | `Attributes_Secondary_BlockChance` | Probability of blocking attacks |
| `Attributes.Secondary.CriticalHitChance` | `Attributes_Secondary_CriticalHitChance` | Probability of critical damage |

## Usage in Attribute System

### Accessing Tags

```cpp
// Get the singleton instance and access tags
const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
FGameplayTag StrengthTag = GameplayTags.Attributes_Primary_Strength;
FGameplayTag ArmorTag = GameplayTags.Attributes_Secondary_Armor;
```

### Integration with Data Assets

These tags serve as keys in the `UAttributeInfo` data asset for attribute metadata lookup:

```cpp
// In AttributeMenuWidgetController
FAuraAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(
    FAuraGameplayTags::Get().Attributes_Primary_Strength
);
```

## Typos Caution and Alternatives

### Risk of String-Based Registration

**Caution**: The `AddNativeGameplayTag` approach uses string literals, making it vulnerable to typos:

```cpp
// Typo risk - string literals not checked at compile time
GameplayTags.Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
    FName("Attributes.Primary.Strenght"), // Typo: "Strenght" instead of "Strength"
    FString("Physical power")
);
```

### Alternative: Macro-Based Tags

Consider using Unreal's `UE_DEFINE_GAMEPLAY_TAG` macro for compile-time safety:

```cpp
// Compile-time checked alternative
UE_DEFINE_GAMEPLAY_TAG(TAG_Attributes_Primary_Strength, "Attributes.Primary.Strength");
```

### Alternative: Code Generation

For large tag sets, consider code generation tools that:
- Read tag definitions from data files (JSON, CSV)
- Generate both the tag strings and singleton member declarations
- Ensure consistency between string names and member names
- Reduce manual typing errors

## Startup Integration

### Asset Manager Connection

The tag initialization is called from `UAuraAssetManager::StartInitialLoading()`:

```cpp
void UAuraAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // Initialize native gameplay tags early in the loading process
    FAuraGameplayTags::InitializeNativeGameplayTags();
    
    // Continue with other initialization...
}
```

This ensures tags are available before any systems attempt to use them.

## Related Documentation

- [Aura Gameplay Tags (Singleton, Native Tags)](./aura-gameplay-tags.md) - General singleton pattern and usage
- [Attribute Info Data Asset](../data/attribute-info.md) - How these tags are used as lookup keys
- [Attribute Menu Widget Controller Setup](../ui/attribute-menu/attribute-menu-widget-controller-setup.md) - Controller usage of attribute tags
- [Gameplay Tags Centralization](./gameplay-tags-centralization.md) - Broader tag management strategies