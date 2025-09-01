# Binding callbacks with UTDEnhancedInputComponent (Ability Inputs)

Last updated: 2025-09-01

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

## Setup checklist (end-to-end)

1) Create/verify your Input Config asset (UTDInputConfig)
- IA_LMB ↔ InputTag.LMB
- IA_RMB ↔ InputTag.RMB
- IA_QuickSlot1..4 ↔ InputTag.QuickSlot1..4

2) Set the default input component class
- Project Settings → Input → Default Input Component Class → select UTDEnhancedInputComponent.

3) Assign the Input Config on your PlayerController Blueprint
- Add UPROPERTY(EditDefaultsOnly) UTDInputConfig* InputConfig to your PlayerController.
- In BP, set InputConfig to your asset (e.g., TDInputConfig).

4) Define three callbacks on the PlayerController
- Include header for FGameplayTag in your PC header if the signatures are in the header file:
  - #include "GameplayTagContainer.h"
- Example signatures:

```cpp
void AbilityInputTagPressed(FGameplayTag InputTag);
void AbilityInputTagReleased(FGameplayTag InputTag);
void AbilityInputTagHeld(FGameplayTag InputTag);
```

5) Bind once in SetupInputComponent

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
```

Notes:
- The order of callback parameters is Pressed, then Released, then Held.
- If your function pointer types are wrappers (delegates), ensure they are valid; the implementation guards with IsValid() before binding.

## Quick verification with on-screen debug messages

Add temporary logging to confirm the correct tag flows through. Using different keys prevents messages from overriding one another:

```cpp
void AMyPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(/*Key*/ 1, /*Time*/ 3.f, FColor::Red,
            FString::Printf(TEXT("Pressed: %s"), *InputTag.ToString()));
    }
}

void AMyPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(/*Key*/ 2, /*Time*/ 3.f, FColor::Blue,
            FString::Printf(TEXT("Released: %s"), *InputTag.ToString()));
    }
}

void AMyPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(/*Key*/ 3, /*Time*/ 3.f, FColor::Green,
            FString::Printf(TEXT("Held: %s"), *InputTag.ToString()));
    }
}
```

Expected behavior when you click LMB or press 1..4:
- Red appears immediately (Pressed)
- Green repeats while held (Held)
- Blue shows when released (Released)

## Blueprint notes

- Expose equivalent handlers in Blueprint that accept a Gameplay Tag and call Try Activate Abilities By Tag on your Ability System Component.
- Ensure your Input Mapping Context is applied and active on the local player.

## Troubleshooting

- Hit a check/assert on InputConfig? Assign the InputConfig asset on your PlayerController BP.
- No callbacks firing? Confirm Project Settings → Input → Default Input Component Class is set to UTDEnhancedInputComponent and your IMC is active.
- Wrong tag? Verify the UTDInputConfig entry's InputTag matches centralized names exactly (InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4).
- Held not repeating? Ensure the binding for Held uses ETriggerEvent::Triggered and the UInputAction isn't gated by triggers/modifiers.

## See also

- Data-driven mapping pattern: Enhanced Input → Ability Mapping
- Centralized tags and conventions: Gameplay Tags Centralization