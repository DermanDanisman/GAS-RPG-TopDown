# Binding callbacks with UTDEnhancedInputComponent (Ability Inputs)

Last updated: 2025-08-31

This guide shows how to bind ability-related input callbacks using UTDEnhancedInputComponent and a data-driven UTDInputConfig. It complements the Enhanced Input → Ability Mapping guide.

Related:
- Enhanced Input → Ability Mapping: ./enhanced-input-to-abilities.md
- Gameplay Tags Centralization: ../../systems/gameplay-tags-centralization.md
- Abilities Overview: ../../gas/abilities/overview.md

## What it is

UTDEnhancedInputComponent derives from UEnhancedInputComponent and exposes a templated helper:

```cpp
// Signature (from TDEnhancedInputComp.h)
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void BindAbilityInputActions(const UTDInputConfig* InputConfig, UserClass* Object, 
                             PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
```

The helper:
- Iterates InputConfig->AbilityInputActions (pairs of UInputAction* + FGameplayTag).
- Binds three callbacks per action using Enhanced Input trigger events:
  - Pressed: ETriggerEvent::Started
  - Released: ETriggerEvent::Completed
  - Held/Repeat: ETriggerEvent::Triggered (fires every frame while held)
- Forwards the entry's InputTag to your callback as an extra parameter.

## Prerequisites

- UTDInputConfig data asset with entries like:
  - IA_LMB ↔ InputTag.LMB
  - IA_RMB ↔ InputTag.RMB
  - IA_QuickSlot1..4 ↔ InputTag.QuickSlot1..4
- Input tags registered centrally via FTDGameplayTags (see Gameplay Tags Centralization).
- Your Input Mapping Context includes and enables the mapped UInputActions.

## Usage in a PlayerController

Example of binding once during input setup and routing to GAS via tags:

```cpp
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UTDEnhancedInputComponent* EIC = CastChecked<UTDEnhancedInputComponent>(InputComponent);
    
    if (InputConfig)
    {
        EIC->BindAbilityInputActions(InputConfig, this, 
                                     &ThisClass::AbilityInputTagPressed,
                                     &ThisClass::AbilityInputTagReleased,
                                     &ThisClass::AbilityInputTagHeld);
    }
}

// Handler examples
void AMyPlayerController::AbilityInputTagPressed(FGameplayTag InputTag) 
{
    if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AMyPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
}

void AMyPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
}
```

Blueprints: You can expose similar handlers that accept a Gameplay Tag and call Try Activate Abilities By Tag on your Ability System Component.

## Tips and troubleshooting

- If callbacks don't fire: verify the Input Mapping Context is active and the UInputActions in your UTDInputConfig are the ones being triggered.
- If your function pointers are wrappers (e.g., delegates), ensure you pass valid ones; the implementation guards with IsValid() before binding.
- Tags must match the centralized names exactly (InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4).
- Held callbacks use Triggered, which repeats while the input is pressed; use Started for single-shot press and Completed for release.

## See also

- Data-driven mapping pattern: Enhanced Input → Ability Mapping
- Centralized tags and conventions: Gameplay Tags Centralization