# Enhanced Input → Ability Mapping (Data-Driven) — Aura Input Config

Last updated: 2025-08-31

Goal: Activate abilities via Enhanced Input without hard-wiring enums or rigid input IDs. Use a data-driven UDataAsset that pairs UInputAction assets with Gameplay Tags (InputTag.*), so mappings can be swapped or edited at runtime.

Related:
- Abilities Overview: ../../gas/abilities/overview.md
- Ability Tags & Policies: ../../gas/abilities/ability-tags-and-policies.md
- Base Ability and Startup Grant: ../../gas/abilities/base-ability-and-startup-grant.md
- Gameplay Effects — Guide: ../../gas/gameplay-effects.md
- Gameplay Tags Centralization: ../../systems/gameplay-tags-centralization.md
- Replication & Multiplayer: ../../architecture/replication-and-multiplayer.md

## Approach overview

- Enhanced Input binds keys/mouse/gamepad to UInputAction assets via an Input Mapping Context.
- A lightweight UDataAsset (AuraInputConfig) stores an array of pairs: [UInputAction*, GameplayTag].
- Each input in your scheme gets a tag in the InputTag.* namespace (e.g., InputTag.LMB, .RMB, .1, .2, .3, .4).
- At runtime, you can:
  - Look up the UInputAction for a given InputTag (e.g., to verify or display bindings),
  - Or, more commonly, look up the InputTag for a triggered UInputAction and route to the corresponding ability.

Lyra does something similar with more indirection. This version keeps the flexibility while staying approachable.

## Data schema (C++)

Struct and data asset (header sketch):

```cpp
// AuraInputConfig.h (header sketch)
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h" // for FGameplayTag
#include "AuraInputConfig.generated.h"

class UInputAction; // forward declare to avoid heavy includes in the header

USTRUCT(BlueprintType)
struct FAuraInputAction
{
    GENERATED_BODY()

    // Input action asset mapped to a specific input (key/mouse/gamepad) via Input Mapping Context
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    const UInputAction* InputAction = nullptr; // const pointer is fine; asset itself won't mutate

    // Tag representing the semantic input (InputTag.*). NOTE: UPROPERTY doesn't support const value types.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag InputTag; // e.g., InputTag.LMB, InputTag.1, etc.
};

UCLASS(BlueprintType)
class UAuraInputConfig : public UDataAsset
{
    GENERATED_BODY()
public:
    // Designer-filled list of semantic input bindings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FAuraInputAction> AbilityInputActions;

    // Helper to find UInputAction by tag (e.g., for UI display of current bindings)
    const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InInputTag, bool bLogNotFound = false) const;

    // Helper to find tag by UInputAction (e.g., for routing triggered inputs)
    FGameplayTag FindAbilityInputTagForAction(const UInputAction* InInputAction, bool bLogNotFound = false) const;
};
```

Implementation sketch:

```cpp
// AuraInputConfig.cpp (impl sketch)
#include "AuraInputConfig.h"
#include "InputAction.h" // minimal include for UInputAction

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InInputTag, bool bLogNotFound) const
{
    for (const FAuraInputAction& Action : AbilityInputActions)
    {
        if (Action.InputAction && Action.InputTag == InInputTag)
        {
            return Action.InputAction;
        }
    }

    if (bLogNotFound)
    {
        UE_LOG(LogTemp, Error, TEXT("AuraInputConfig: No InputAction for tag %s on %s"),
               *InInputTag.ToString(), *GetNameSafe(this));
    }
    return nullptr;
}

FGameplayTag UAuraInputConfig::FindAbilityInputTagForAction(const UInputAction* InInputAction, bool bLogNotFound) const
{
    for (const FAuraInputAction& Action : AbilityInputActions)
    {
        if (Action.InputAction == InInputAction)
        {
            return Action.InputTag;
        }
    }

    if (bLogNotFound)
    {
        UE_LOG(LogTemp, Error, TEXT("AuraInputConfig: No InputTag for action %s on %s"),
               *GetNameSafe(InInputAction), *GetNameSafe(this));
    }
    return FGameplayTag();
}
```

Notes:
- UPROPERTY does not support const for value types like FGameplayTag; remove const on the tag field.
- Using a const UInputAction* is fine since assets are immutable at runtime.

## Gameplay Tags for inputs (InputTag.*)

Define a dedicated category in your centralized tags:
- InputTag.LMB — Left Mouse Button
- InputTag.RMB — Right Mouse Button
- InputTag.1, InputTag.2, InputTag.3, InputTag.4 — Number keys 1–4

Register natively in your Tags singleton (example):

```cpp
AddNativeGameplayTag(GameplayTags.InputTag_LMB, FName("InputTag.LMB"), TEXT("Input tag for Left Mouse Button"));
AddNativeGameplayTag(GameplayTags.InputTag_RMB, FName("InputTag.RMB"), TEXT("Input tag for Right Mouse Button"));
AddNativeGameplayTag(GameplayTags.InputTag_1,   FName("InputTag.1"),   TEXT("Input tag for 1 key"));
AddNativeGameplayTag(GameplayTags.InputTag_2,   FName("InputTag.2"),   TEXT("Input tag for 2 key"));
AddNativeGameplayTag(GameplayTags.InputTag_3,   FName("InputTag.3"),   TEXT("Input tag for 3 key"));
AddNativeGameplayTag(GameplayTags.InputTag_4,   FName("InputTag.4"),   TEXT("Input tag for 4 key"));
```

See: Gameplay Tags Centralization for the project's canonical registration site.

## Editor setup (step-by-step)

1) Create UInputAction assets
- IA_LMB, IA_RMB, IA_1, IA_2, IA_3, IA_4
- Value Type: Axis1D (float) for button-like inputs.

2) Map in your Input Mapping Context
- Add mappings: IA_1→Key 1, IA_2→Key 2, IA_3→Key 3, IA_4→Key 4, IA_LMB→Mouse Left, IA_RMB→Mouse Right.

3) Create the Aura Input Config data asset
- Assets → Miscellaneous → Data Asset → AuraInputConfig
- Fill AbilityInputActions with pairs:
  - IA_LMB ↔ InputTag.LMB
  - IA_RMB ↔ InputTag.RMB
  - IA_1 ↔ InputTag.1, … up to IA_4 ↔ InputTag.4

Tip: You can swap the entire data asset at runtime to change bindings for modes/classes without code changes.

## Runtime usage patterns

There are two practical routing strategies; pick one and stick to it.

A) Route by InputTag assigned to Abilities
- Give each activatable ability an Ability Tag that matches the input tag (e.g., Ability.Input.LMB).
- On Enhanced Input trigger:
  - Convert UInputAction→InputTag via the data asset (or store a reverse map),
  - Query the ASC for an ability matching that input tag and TryActivate.

B) Route by direct InputTag on the ability (no reverse map)
- Store the InputTag in the ability (Ability Tags: Ability.Input.*) and drive UI/behavior from tags.
- For UI, display current binding for a tag with FindAbilityInputActionForTag(tag).

Example (pseudo C++):

```cpp
void UMyInputRouter::OnInputTriggered(const UInputAction* TriggeredAction)
{
    // Option 1: Reverse lookup tag from action (add a helper if you want)
    FGameplayTag InputTag;
    for (const FAuraInputAction& Pair : AuraInputConfig->AbilityInputActions)
    {
        if (Pair.InputAction == TriggeredAction) { InputTag = Pair.InputTag; break; }
    }
    if (!InputTag.IsValid()) return;

    // Option 2: If you derived tag another way, use it here
    AbilitySystem->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}
```

Blueprint equivalent: keep the data asset accessible on the PlayerController or Pawn, loop the array to find the tag, then "Try Activate Abilities By Tag".

## Testing and troubleshooting

- showdebug abilitysystem: verify activations and blocked states.
- If the lookup returns nullptr:
  - Ensure the InputTag exists in the data asset and matches your centralized tags exactly.
  - Confirm the Input Mapping Context includes and enables the mapped UInputActions.
- Const pitfalls:
  - Do not mark FGameplayTag UPROPERTY fields as const; compilation will fail.
  - A const UInputAction* UPROPERTY is acceptable; it references an immutable asset.

## Pitfalls and tips

- Don't hardcode input enums in abilities; use tags so bindings can change without code.
- Keep InputTag.* distinct from Ability.* tags; use a clear namespace like Ability.Input.* inside abilities if desired.
- Consider a reverse lookup helper: FindTagForInputAction(UInputAction*) for convenience.
- For multiplayer, ensure activation routing happens on the owning client with proper prediction (see Replication & Multiplayer).