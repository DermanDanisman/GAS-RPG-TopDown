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
- Fallback: Asset manager's StartInitialLoading()

Example (module):

```cpp
void FRPG_TopdownModule::StartupModule()
{
    FTDGameplayTags::InitializeNativeGameplayTags();
}
```

Example (asset manager - this repo uses this approach):

```cpp
void UTDAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // Register all native gameplay tags used by the project before gameplay begins.
    FTDGameplayTags::InitializeNativeGameplayTags();
}
```

## Module initialization order
1. Engine modules load first
2. Game module StartupModule() called 
3. Asset manager StartInitialLoading() called (if custom)
4. GameInstance Init() called
5. World begins play

Since tags are used throughout gameplay, initialize as early as possible (module or asset manager preferred).

## Validation after initialization
Verify tags are properly registered in Editor:
1. Project Settings → Gameplay Tags
2. Search for "InputTag" - should show InputTag.LMB, InputTag.RMB, etc.
3. Search for "Attributes" - should show Primary/Secondary/Vital hierarchies

If tags don't appear, check:
- InitializeNativeGameplayTags() is being called
- No early returns or exceptions in the initialization function
- Module dependencies are correct

## Getting tags in code

Use the singleton accessor to avoid hardcoded strings:

```cpp
const FTDGameplayTags& Tags = FTDGameplayTags::Get();
FGameplayTag LMB = Tags.InputTag_LMB; // "InputTag.LMB"
```

## Access patterns by use case

### In gameplay abilities
```cpp
void UMyGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, ...)
{
    const FTDGameplayTags& Tags = FTDGameplayTags::Get();
    
    // Check if ability was triggered by specific input
    FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
    if (Spec && Spec->GetDynamicSpecSourceTags().HasTagExact(Tags.InputTag_LMB))
    {
        // Left mouse button activation logic
    }
}
```

### In controllers and components
```cpp
void ATDPlayerController::AbilityInputActionHeld(FGameplayTag InputTag)
{
    // Direct comparison with centralized tags
    const FTDGameplayTags& Tags = FTDGameplayTags::Get();
    
    if (InputTag == Tags.InputTag_LMB)
    {
        // Special handling for LMB
    }
    
    // Forward to ASC
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}
```

### In data assets and configuration
```cpp
// When setting up UTDInputConfig programmatically
void UTDInputConfig::SetupDefaultBindings()
{
    const FTDGameplayTags& Tags = FTDGameplayTags::Get();
    
    // Add LMB binding
    FGASCoreAbilityInputAction LMBAction;
    LMBAction.InputAction = LoadObject<UInputAction>(nullptr, TEXT("/Path/To/IA_LMB"));
    LMBAction.InputTag = Tags.InputTag_LMB;
    AbilityInputActions.Add(LMBAction);
}
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