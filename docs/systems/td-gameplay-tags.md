# TD Gameplay Tags (Singleton, Native Tags)

Last updated: 2024-12-19

## Goal

Centralize gameplay tag access in C++ and expose a single source of truth for all gameplay tags through the `FTDGameplayTags` singleton pattern. This provides compile-time safety, performance benefits, and eliminates scattered `RequestGameplayTag` calls throughout the codebase.

## Why Centralize Gameplay Tags

### Problems with Scattered RequestGameplayTag Calls

Using `RequestGameplayTag` with string literals throughout the codebase creates several issues:

```cpp
// Problematic: Scattered throughout different files
FGameplayTag StrengthTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Attributes.Primary.Strength"));
FGameplayTag ArmorTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Attributes.Secondary.Armor"));
```

**Issues:**
- **Runtime Overhead**: String-based lookups every time you need a tag
- **Typo-Prone**: String literals vulnerable to spelling errors
- **No Compile-Time Safety**: Errors only discovered at runtime
- **Maintenance Difficulty**: Hard to refactor tag names across the codebase
- **No IDE Support**: No auto-completion or go-to-definition

### Benefits of Centralization

- **Single Source of Truth**: All tags defined in one location
- **Compile-Time Safety**: Reference tags as typed members
- **Performance**: Pre-resolved tags avoid runtime lookups
- **IDE Integration**: Full autocomplete and navigation support
- **Easy Refactoring**: Change tag names in one place

## Gameplay Tag Creation Options

### Option 1: UE_DEFINE_GAMEPLAY_TAG Macro

Unreal's built-in macro for compile-time tag definition:

```cpp
// In header file
UE_DEFINE_GAMEPLAY_TAG(TAG_Attributes_Primary_Strength, "Attributes.Primary.Strength");

// Usage
FGameplayTag StrengthTag = TAG_Attributes_Primary_Strength;
```

**Pros:**
- Compile-time validation
- Native source flag automatically set
- No runtime initialization required

**Cons:**
- Tags scattered across multiple files
- Harder to organize and maintain
- Limited Blueprint integration

### Option 2: UGameplayTagsManager::AddNativeGameplayTag

Runtime registration of native tags through the gameplay tag manager:

```cpp
void FTDGameplayTags::InitializeNativeGameplayTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
    
    TDTDFTDGameplayTags::Get().Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Strength"),
        FString("Physical power attribute")
    );
}
```

**Pros:**
- Centralized organization
- Runtime descriptions
- Better Blueprint integration
- Flexible initialization

**Cons:**
- Requires runtime initialization
- Must ensure proper startup order

## FTDGameplayTags Singleton Pattern

### Header Definition (FTDTDGameplayTags.h)

```cpp
USTRUCT(BlueprintType)
struct GASCORE_API FTDGameplayTags
{
    GENERATED_BODY()

    // Singleton access - returns the single instance
    static const FTDGameplayTags& Get() { return GameplayTags; }
    
    // Initialize all native gameplay tags - call once at startup
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

    // Add more tags as needed...

private:
    // Single static instance
    static FTDGameplayTags GameplayTags;
};
```

### Implementation (FTDTDGameplayTags.cpp)

```cpp
// Static instance definition
FTDGameplayTags FTDGameplayTags::GameplayTags;

void FTDGameplayTags::InitializeNativeGameplayTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // Primary Attributes - AddNativeGameplayTag returns the created tag
    TDFTDGameplayTags::Get().Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Strength"),
        FString("Physical power attribute")
    );

    TDFTDGameplayTags::Get().Attributes_Primary_Dexterity = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Dexterity"),
        FString("Agility and precision attribute")
    );

    TDFTDGameplayTags::Get().Attributes_Primary_Intelligence = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Intelligence"),
        FString("Mental acuity and mana capacity")
    );

    TDFTDGameplayTags::Get().Attributes_Primary_Willpower = Manager.AddNativeGameplayTag(
        FName("Attributes.Primary.Willpower"),
        FString("Mental fortitude and mana regeneration")
    );

    // Secondary Attributes
    TDFTDGameplayTags::Get().Attributes_Secondary_Armor = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.Armor"),
        FString("Physical damage reduction")
    );

    TDFTDGameplayTags::Get().Attributes_Secondary_ArmorPenetration = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.ArmorPenetration"),
        FString("Armor bypass capability")
    );

    TDFTDGameplayTags::Get().Attributes_Secondary_BlockChance = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.BlockChance"),
        FString("Probability of blocking attacks")
    );

    TDFTDGameplayTags::Get().Attributes_Secondary_CriticalHitChance = Manager.AddNativeGameplayTag(
        FName("Attributes.Secondary.CriticalHitChance"),
        FString("Probability of critical strikes")
    );

    // Continue for all attributes...
}
```

## Usage Examples

### Accessing Tags in Other Classes

```cpp
// In any C++ class - clean, compile-time safe access
void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    const UTDAttributeSet* CoreAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    // Use the singleton to get tag references
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetStrengthAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
            HandleAttributeChanged(FTDGameplayTags::Get().Attributes_Primary_Strength, Data.NewValue);
        });
        
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetArmorAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
            HandleAttributeChanged(FTDGameplayTags::Get().Attributes_Secondary_Armor, Data.NewValue);
        });
}
```

### Printing to Screen Example

```cpp
void AMyActor::PrintAttributeInfo()
{
    // Access centralized tags and print to screen
    const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();
    
    FString TagName = TDFTDGameplayTags::Get().Attributes_Secondary_Armor.ToString();
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
        FString::Printf(TEXT("Armor Tag: %s"), *TagName));
}
```

### Blueprint Integration

The `UPROPERTY(BlueprintReadOnly)` tags make the centralized tags available in Blueprints:

```cpp
// Blueprint nodes can access:
// - Get Aura Gameplay Tags -> Attributes Primary Strength
// - Tag comparison and container operations
// - Data Asset lookups using centralized tags
```

## Native vs Config/Data-Table Tags

### When to Use Native Tags

**Use `AddNativeGameplayTag` for:**
- Core gameplay systems (attributes, abilities, effects)
- Tags that need compile-time references in C++
- Performance-critical tag operations
- Tags that rarely change during development

**Benefits:**
- Native source flag set automatically
- Better performance (pre-resolved)
- Compile-time safety in C++ code
- Automatic Blueprint integration

### When to Use Config/Data-Table Tags

**Use config files or data tables for:**
- Content-specific tags (item types, quest states)
- Tags that designers need to modify
- Localization-dependent tag descriptions
- Tags that change frequently during development

**Example Config (DefaultTDGameplayTags.ini):**
```ini
[/Script/TDGameplayTags.GameplayTagsList]
GameplayTagList=(Tag="Items.Weapons.Sword",DevComment="Sword weapon type")
GameplayTagList=(Tag="Items.Armor.Helmet",DevComment="Helmet armor piece")
```

## Compile-Time References and Native Source Flag

### Native Source Flag

When using `AddNativeGameplayTag`, the tag automatically receives the "Native" source flag:

- **Editor Integration**: Tags appear with "Native" source in tag editors
- **Performance**: Native tags have optimized lookup paths
- **Validation**: Native tags are validated at startup
- **Debugging**: Easy to identify programmatically-created tags

### Compile-Time References

With the macro approach (`UE_DEFINE_GAMEPLAY_TAG`):

```cpp
// Compile-time validation - typos caught at compile time
UE_DEFINE_GAMEPLAY_TAG(TAG_Attributes_Primary_Strength, "Attributes.Primary.Strength");

// vs

// Runtime validation - typos only caught at runtime
TDFTDGameplayTags::Get().Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
    FName("Attributes.Primary.Strength"), // Typo here would only be caught at runtime
    FString("Physical power attribute")
);
```

The singleton approach trades some compile-time safety for better organization and Blueprint integration.

## Related Documentation

- [Aura Asset Manager](asset-manager.md) - How to initialize native gameplay tags at startup
- [Gameplay Tags Centralization](gameplay-tags-centralization.md) - Broader centralization concepts and migration strategies
- [Attribute Menu Widget Controller](../ui/attribute-menu/attribute-menu-controller.md) - Usage in UI systems