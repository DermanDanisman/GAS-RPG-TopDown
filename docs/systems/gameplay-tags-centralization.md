# Gameplay Tags Centralization (Native)

Last updated: 2025-09-02

This project centralizes all gameplay tags in a single native registry for consistency and discoverability.

## Native registry: FTDGameplayTags

Location: Public/System code (FTDGameplayTags.h / TDGameplayTags2.cpp)

```cpp
// FTDGameplayTags.h (summary)
struct FTDGameplayTags
{
    static const FTDGameplayTags& Get();
    static void InitializeNativeGameplayTags();
    // Attributes (Primary/Secondary/Vital) and Inputs (InputTag.*)
    FGameplayTag InputTag_LMB, InputTag_RMB;
    FGameplayTag InputTag_QuickSlot_1, InputTag_QuickSlot_2, InputTag_QuickSlot_3, InputTag_QuickSlot_4;
    // ... see file for full list
};
```

Initialization registers all tags with UGameplayTagsManager:

```cpp
// TDGameplayTags2.cpp (excerpt)
void FTDGameplayTags::InitializeNativeGameplayTags()
{
    TDGameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.LMB")), TEXT("Input Tag for Left Mouse Button"));
    TDGameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.RMB")), TEXT("Input Tag for Right Mouse Button"));
    TDGameplayTags.InputTag_QuickSlot_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot1")), TEXT("Input Tag for 1 key"));
    TDGameplayTags.InputTag_QuickSlot_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot2")), TEXT("Input Tag for 2 key"));
    TDGameplayTags.InputTag_QuickSlot_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot3")), TEXT("Input Tag for 3 key"));
    TDGameplayTags.InputTag_QuickSlot_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot4")), TEXT("Input Tag for 4 key"));
    // Primary/Secondary/Vital attributes also registered here
}
```

## When to initialize

Call once at startup before any tags are used:

- Preferred: In your game module's StartupModule()
- Alternative: Early in GameInstance::Init()

Example (module):

```cpp
void FRPG_TopdownModule::StartupModule()
{
    FTDGameplayTags::InitializeNativeGameplayTags();
}
```

## Getting tags in code

Use the singleton accessor to avoid hardcoded strings:

```cpp
const FTDGameplayTags& Tags = FTDGameplayTags::Get();
FGameplayTag LMB = Tags.InputTag_LMB; // "InputTag.LMB"
```

## Input tags and Enhanced Input mapping

- The canonical input tags are:
  - InputTag.LMB, InputTag.RMB
  - InputTag.QuickSlot1, InputTag.QuickSlot2, InputTag.QuickSlot3, InputTag.QuickSlot4
- In UTDInputConfig (extends UGASCoreAbilityInputConfig), your entries map UInputAction assets to these tags.
- FindAbilityInputActionByTag uses MatchesTag to support hierarchy; our InputTag.* are peers under the InputTag family.

## Activation matching (exact)

- In the ASC (UGASCoreAbilitySystemComponent), input routing uses HasTagExact on Spec.GetDynamicSpecSourceTags().
- Ensure the ability's dynamic tags contain the exact InputTag.* value to match activation.

## Attributes families (for reference)

- Attributes.Primary.*: Strength, Dexterity, Intelligence, Endurance, Vigor
- Attributes.Secondary.*: Armor, ArmorPenetration, BlockChance, Evasion, CriticalHitChance, CriticalHitDamage, CriticalHitResistance, MaxHealth, HealthRegeneration, MaxMana, ManaRegeneration, StaminaRegeneration, MaxStamina
- Attributes.Vital.*: Health, Mana, Stamina (and project-specific Max* where needed)

## Adding new tags

- Prefer extending FTDGameplayTags and registering via AddNativeGameplayTag in InitializeNativeGameplayTags().
- Keep naming consistent (use family parents like InputTag.*, Attributes.Primary.*, etc.).
- After adding, recompile and ensure initialization runs at startup.

## Common pitfalls

- Not calling InitializeNativeGameplayTags → lookups may fail or tags appear unset at runtime.
- Typos in tag names (e.g., "InputTag.QuickSlot-1"): use the centralized fields via FTDGameplayTags::Get() to avoid string literals.
- Using MatchesTag in ASC activation: use HasTagExact for precise matching to avoid accidental parent matches.