# Binding callbacks with UTDEnhancedInputComponent (Ability Inputs)

Last updated: 2025-09-02

This guide shows how to bind ability-related input callbacks using UTDEnhancedInputComponent and a data-driven UTDInputConfig. It complements the Enhanced Input → Ability Mapping and the Ability Input Tags and Activation guides.

Related:
- Enhanced Input → Ability Mapping: ./enhanced-input-to-abilities.md
- Ability Input Tags and Activation: ../abilities/ability-input-tags-and-activation.md
- Gameplay Tags Centralization: ../../systems/gameplay-tags-centralization.md

## Recap: what the helper does

```cpp
// Signature (from TDEnhancedInputComp.h)
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void BindAbilityInputActions(const UTDInputConfig* InputConfig, UserClass* Object, 
                             PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
```

- Loops InputConfig->AbilityInputActions (UInputAction* + FGameplayTag pairs)
- Binds:
  - Pressed → ETriggerEvent::Started
  - Released → ETriggerEvent::Completed
  - Held/Repeat → ETriggerEvent::Triggered
- Forwards the entry's InputTag to your callbacks as an extra parameter

## Setup checklist

1) Input Config (UTDInputConfig)
- IA_LMB ↔ InputTag.LMB; IA_RMB ↔ InputTag.RMB; IA_QuickSlot1..4 ↔ InputTag.QuickSlot1..4

2) Project Settings
- Input → Default Input Component Class → UTDEnhancedInputComponent

3) PlayerController Blueprint
- UPROPERTY(EditDefaultsOnly, Category=Input) UTDInputConfig* InputConfig (assign your asset)

4) PlayerController callbacks and binding

```cpp
// Header may need GameplayTagContainer.h for FGameplayTag
void AbilityInputTagPressed(FGameplayTag InputTag);
void AbilityInputTagReleased(FGameplayTag InputTag);
void AbilityInputTagHeld(FGameplayTag InputTag);

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

Order matters: Pressed, Released, then Held.

## Caching the ASC (recommended)

Avoid casting every frame during Held by caching the ASC once it becomes valid:

```cpp
// Forward declare UAuraAbilitySystemComponent; store a TObjectPtr<UAuraAbilitySystemComponent> CachedASC;

UAuraAbilitySystemComponent* GetASC()
{
    if (CachedASC.IsValid())
    {
        return CachedASC.Get();
    }
    if (GetPawn())
    {
        CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
    }
    return CachedASC.Get();
}

void AbilityInputTagHeld(FGameplayTag InputTag)
{
    if (UAuraAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}
```

## Quick verification (on-screen debug)

Use distinct keys/colors so messages don't overwrite each other:

```cpp
if (GEngine) GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, FString::Printf(TEXT("Pressed: %s"), *InputTag.ToString()));
if (GEngine) GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, FString::Printf(TEXT("Released: %s"), *InputTag.ToString()));
if (GEngine) GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Green, FString::Printf(TEXT("Held: %s"), *InputTag.ToString()));
```

## Troubleshooting
- Ensure Default Input Component Class is UTDEnhancedInputComponent
- Assign InputConfig on the PlayerController BP
- Input Mapping Context must be applied/active
- Use exact tag names (InputTag.LMB/RMB/QuickSlot1..4)
- Guard null ASC in early frame calls