# Binding callbacks with UTDEnhancedInputComponent (Ability Inputs)

Last updated: 2025-09-02

This guide shows how to bind ability-related input callbacks using UTDEnhancedInputComponent (extends GASCore's UGASCoreEnhancedInputComponent) and a data-driven UTDInputConfig (extends UGASCoreAbilityInputConfig). It complements the Enhanced Input → Ability Mapping and the Ability Input Tags and Activation guides.

Related:
- Enhanced Input → Ability Mapping: ./enhanced-input-to-abilities.md
- Ability Input Tags and Activation: ../abilities/ability-input-tags-and-activation.md
- Gameplay Tags Centralization: ../../systems/gameplay-tags-centralization.md

## Input tags used by this project

From FTDGameplayTags (centralized):
- InputTag.LMB, InputTag.RMB
- InputTag.QuickSlot1, InputTag.QuickSlot2, InputTag.QuickSlot3, InputTag.QuickSlot4

Access in code:

```cpp
const FTDGameplayTags& Tags = FTDGameplayTags::Get();
// Example: pass exact LMB to ASC
ASC->AbilityInputTagHeld(Tags.InputTag_LMB);
```

Notes:
- UTDInputConfig lookup uses MatchesTag (hierarchy-friendly)
- ASC activation uses HasTagExact (precise match on Spec.GetDynamicSpecSourceTags())

## What the helper does

```cpp
// Signature (from GASCoreEnhancedInputComponent.h)
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void BindAbilityInputActions(const UGASCoreAbilityInputConfig* InputConfig, UserClass* Object, 
                             PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
```

- Loops InputConfig->AbilityInputActions (UInputAction + FGameplayTag pairs)
- Binds on Enhanced Input events:
  - Pressed → ETriggerEvent::Started
  - Released → ETriggerEvent::Completed
  - Held/Repeat → ETriggerEvent::Triggered (fires every tick while held)
- Forwards the entry's InputTag to your callbacks as an extra parameter

Implementation notes from GASCore:
- Function pointer null checks use simple pointer checks (PressedFunc/ReleasedFunc/HeldFunc), not .IsValid().
- UTDInputConfig overrides UGASCoreAbilityInputConfig and inherits FindAbilityInputActionByTag behavior (MatchesTag).

## Setup checklist

1) Input Config
- Use UTDInputConfig (data asset) and add entries for your actions ↔ tags (e.g., InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4)

2) Project Settings
- Input → Default Input Component Class → UTDEnhancedInputComponent

3) PlayerController Blueprint
- Assign on ATDPlayerController:
  - GASInputMappingContext (UInputMappingContext)
  - MoveAction (UInputAction)
  - InputConfig (UTDInputConfig)

4) Mapping context setup (BeginPlay)

```cpp
void ATDPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (const ULocalPlayer* LP = Cast<ULocalPlayer>(GetLocalPlayer()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            EISubsystem->AddMappingContext(GASInputMappingContext, 0);
        }
    }
}
```

5) Bind once in SetupInputComponent

```cpp
void ATDPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UTDEnhancedInputComponent* EIC = CastChecked<UTDEnhancedInputComponent>(InputComponent);

    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDPlayerController::Move);

    if (InputConfig)
    {
        EIC->BindAbilityInputActions(InputConfig, this, 
                                     &ThisClass::AbilityInputActionTagPressed,
                                     &ThisClass::AbilityInputActionReleased,
                                     &ThisClass::AbilityInputActionTagHeld);
    }
}
```

6) Callback signatures and ASC forwarding

```cpp
// Header may need GameplayTagContainer.h for FGameplayTag
void AbilityInputActionTagPressed(FGameplayTag InputTag);
void AbilityInputActionReleased(FGameplayTag InputTag);
void AbilityInputActionHeld(FGameplayTag InputTag);

UTDAbilitySystemComponent* GetASC()
{
    if (!TDAbilitySystemComponent)
    {
        TDAbilitySystemComponent = Cast<UTDAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
    }
    return TDAbilitySystemComponent;
}
```

## Quick verification (on-screen debug)

Use distinct keys/colors so messages don't overwrite each other:

```cpp
if (GEngine) GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red,   FString::Printf(TEXT("Pressed: %s"),  *InputTag.ToString()));
if (GEngine) GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue,  FString::Printf(TEXT("Released: %s"), *InputTag.ToString()));
if (GEngine) GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Green, FString::Printf(TEXT("Held: %s"),     *InputTag.ToString()));
```

## Troubleshooting
- Default Input Component Class must be UTDEnhancedInputComponent
- GASInputMappingContext must be assigned and added in BeginPlay
- MoveAction and InputConfig must be set in BP/defaults
- Tags must match centralized names from FTDGameplayTags (InputTag.LMB/RMB/QuickSlot1..4)
- Guard null ASC in early frame calls