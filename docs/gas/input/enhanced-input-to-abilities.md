# Enhanced Input → Ability Mapping (Data-Driven) — TD Input Config

Last updated: 2025-08-31

[Note] A ready-made component helper exists: see Binding callbacks with UTDEnhancedInputComponent for a one-call setup that binds Pressed/Released/Held and forwards the InputTag to your handlers.

Reminders for this project:
- Set Project Settings → Input → Default Input Component Class to UTDEnhancedInputComponent.
- Assign your UTDInputConfig asset on the PlayerController Blueprint before running.
- Input tags: InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4 (defined in FTDGameplayTags)

Goal: Activate abilities via Enhanced Input without hard-wiring enums or rigid input IDs. Use a data-driven UDataAsset that pairs UInputAction assets with Gameplay Tags (InputTag.*), so mappings can be swapped or edited at runtime.

Related:
- Binding callbacks with UTDEnhancedInputComponent: ./td-enhanced-input-component.md
- Abilities Overview: ../../gas/abilities/overview.md
- Ability Tags & Policies: ../../gas/abilities/ability-tags-and-policies.md
- Base Ability and Startup Grant: ../../gas/abilities/base-ability-and-startup-grant.md
- Gameplay Effects — Guide: ../../gas/gameplay-effects.md
- Gameplay Tags Centralization: ../../systems/gameplay-tags-centralization.md
- Replication & Multiplayer: ../../architecture/replication-and-multiplayer.md

## Approach overview

- Enhanced Input binds keys/mouse/gamepad to UInputAction assets via an Input Mapping Context.
- A lightweight UDataAsset (UTDInputConfig) stores an array of pairs: [UInputAction*, GameplayTag].
- Each input in your scheme gets a tag in the InputTag.* namespace (e.g., InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4).
- At runtime, you can:
  - Look up the UInputAction for a given InputTag (e.g., to verify or display bindings),
  - Or, more commonly, look up the InputTag for a triggered UInputAction and route to the corresponding ability.

Lyra does something similar with more indirection. This version keeps the flexibility while staying approachable.

## Data schema (C++)

Struct and data asset (header sketch aligned with UTDInputConfig):

```cpp
// TDInputConfig.h (header sketch)
USTRUCT(BlueprintType)
struct FTDInputAction
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category="Input")
    const class UInputAction* InputAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    FGameplayTag InputTag = FGameplayTag();
};

UCLASS()
class UTDInputConfig : public UDataAsset
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    const UInputAction* FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TArray<FTDInputAction> AbilityInputActions;
};
```

Implementation sketch (MatchesTag as in code):

```cpp
// TDInputConfig.cpp (impl sketch)
const UInputAction* UTDInputConfig::FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
    if (!InputTag.IsValid() || AbilityInputActions.IsEmpty())
    {
        if (bLogNotFound)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid tag or empty config on %s"), *GetNameSafe(this));
        }
        return nullptr;
    }

    for (const FTDInputAction& Pair : AbilityInputActions)
    {
        if (Pair.InputAction && Pair.InputTag.MatchesTag(InputTag))
        {
            return Pair.InputAction;
        }
    }

    if (bLogNotFound)
    {
        UE_LOG(LogTemp, Error, TEXT("Cant find Ability Input Action for InputTag [%s] on %s"), *InputTag.ToString(), *GetNameSafe(this));
    }
    return nullptr;
}
```

Notes:
- UPROPERTY does not support const for value types like FGameplayTag; keep it non-const (as in code).
- A const UInputAction* UPROPERTY is fine; assets are immutable at runtime.

## Gameplay Tags for inputs (InputTag.*)

This project centralizes input tags in FTDGameplayTags and registers them natively in TDGameplayTags::InitializeNativeGameplayTags:

```cpp
// Examples (native registration)
UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.LMB")),        TEXT("Input Tag for Left Mouse Button"));
UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.RMB")),        TEXT("Input Tag for Right Mouse Button"));
UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot1")), TEXT("Input Tag for 1 key"));
UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot2")), TEXT("Input Tag for 2 key"));
UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot3")), TEXT("Input Tag for 3 key"));
UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("InputTag.QuickSlot4")), TEXT("Input Tag for 4 key"));
```

Guideline: Prefer InputTag.QuickSlot1..4 over numeric root tags (InputTag.1..4). Keep input tags distinct from ability grouping tags; use Ability.Input.* inside abilities if desired.

## Editor setup (step-by-step)

1) Create UInputAction assets
- IA_LMB, IA_RMB, IA_QuickSlot1, IA_QuickSlot2, IA_QuickSlot3, IA_QuickSlot4
- Value Type: Axis1D (float) for button-like inputs.

2) Map in your Input Mapping Context
- Add mappings: IA_QuickSlot1→Key 1, IA_QuickSlot2→Key 2, IA_QuickSlot3→Key 3, IA_QuickSlot4→Key 4, IA_LMB→Mouse Left, IA_RMB→Mouse Right.

3) Create the Input Config data asset
- Assets → Miscellaneous → Data Asset → UTDInputConfig
- Fill AbilityInputActions with pairs:
  - IA_LMB ↔ InputTag.LMB
  - IA_RMB ↔ InputTag.RMB
  - IA_QuickSlot1 ↔ InputTag.QuickSlot1, … up to IA_QuickSlot4 ↔ InputTag.QuickSlot4

Tip: You can swap the entire data asset at runtime to change bindings for modes/classes without code changes.

## Runtime usage patterns

Two practical routing strategies; pick one and stick to it.

A) Route by InputTag assigned to Abilities
- Give each activatable ability an Ability Tag that matches the input tag (e.g., Ability.Input.QuickSlot1).
- On Enhanced Input trigger:
  - Convert UInputAction→InputTag via the data asset (or store a reverse map),
  - Query the ASC for an ability matching that input tag and TryActivate.

B) Route by direct InputTag on the ability (no reverse map)
- Store the InputTag in the ability (Ability Tags: Ability.Input.*) and drive UI/behavior from tags.
- For UI, display current binding for a tag with FindAbilityInputActionByTag(tag).

Example (pseudo C++):

```cpp
void UMyInputRouter::OnInputTriggered(const UInputAction* TriggeredAction)
{
    FGameplayTag InputTag;
    for (const FTDInputAction& Pair : InputConfig->AbilityInputActions)
    {
        if (Pair.InputAction == TriggeredAction) { InputTag = Pair.InputTag; break; }
    }
    if (!InputTag.IsValid()) return;

    AbilitySystem->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}
```

Blueprint equivalent: keep the data asset accessible on the PlayerController or Pawn, loop the array to find the tag, then "Try Activate Abilities By Tag".

## Testing and troubleshooting

- showdebug abilitysystem: verify activations and blocked states.
- If the lookup returns nullptr:
  - Ensure the InputTag exists in the data asset and matches your centralized tags exactly (QuickSlot names).
  - Confirm the Input Mapping Context includes and enables the mapped UInputActions.
- Const pitfalls:
  - Do not mark FGameplayTag UPROPERTY fields as const; compilation will fail.
  - A const UInputAction* UPROPERTY is acceptable; it references an immutable asset.

## Pitfalls and tips

- Don't hardcode input enums in abilities; use tags so bindings can change without code.
- Keep InputTag.* distinct from Ability.* tags; use a clear namespace like Ability.Input.* inside abilities if desired.
- Consider a reverse lookup helper: FindTagForInputAction(UInputAction*) for convenience when routing from action to tag.
- For multiplayer, ensure activation routing happens on the owning client with proper prediction (see Replication & Multiplayer).