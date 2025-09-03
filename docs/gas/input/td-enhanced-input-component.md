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

**Binding logic:**
- Loops InputConfig->AbilityInputActions (UInputAction + FGameplayTag pairs)
- Binds on Enhanced Input events:
  - Pressed → ETriggerEvent::Started
  - Released → ETriggerEvent::Completed  
  - Held/Repeat → ETriggerEvent::Triggered (fires every tick while held)
- Forwards the entry's InputTag to your callbacks as an extra parameter

**Enhanced Input event mapping:**
- `Started`: Triggered when input first becomes active (key down, mouse press)
- `Triggered`: Triggered every frame while input remains active (continuous hold)  
- `Completed`: Triggered when input becomes inactive (key up, mouse release)

**Implementation details:**
- Function pointer null checks use simple pointer checks (PressedFunc/ReleasedFunc/HeldFunc), not .IsValid()
- UTDInputConfig overrides UGASCoreAbilityInputConfig and inherits FindAbilityInputActionByTag behavior (MatchesTag)
- Each bound callback receives the InputTag from the config entry, not the raw input event
- Bindings persist until component is destroyed or explicitly unbound

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

// Implementation examples
void ATDPlayerController::AbilityInputActionTagPressed(FGameplayTag InputTag)
{
    // Optional: Handle press-specific logic here
    UE_LOG(LogTemp, Log, TEXT("Ability input pressed: %s"), *InputTag.ToString());
}

void ATDPlayerController::AbilityInputActionReleased(FGameplayTag InputTag)
{
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagReleased(InputTag);
    }
}

void ATDPlayerController::AbilityInputActionHeld(FGameplayTag InputTag)
{
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}
```

**Key points:**
- Pressed callback is optional - many setups only use Held/Released
- Always null-check the ASC before forwarding calls  
- GetASC() caches the component reference for performance
- InputTag parameter contains the exact tag from your UTDInputConfig

## Verification steps

### Quick on-screen debug
Use distinct keys/colors so messages don't overwrite each other:

```cpp
void ATDPlayerController::AbilityInputActionTagPressed(FGameplayTag InputTag)
{
    if (GEngine) 
    {
        GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, 
            FString::Printf(TEXT("Pressed: %s"), *InputTag.ToString()));
    }
}

void ATDPlayerController::AbilityInputActionReleased(FGameplayTag InputTag)
{
    if (GEngine) 
    {
        GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, 
            FString::Printf(TEXT("Released: %s"), *InputTag.ToString()));
    }
    
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagReleased(InputTag);
    }
}

void ATDPlayerController::AbilityInputActionHeld(FGameplayTag InputTag)
{
    if (GEngine) 
    {
        GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Green, 
            FString::Printf(TEXT("Held: %s"), *InputTag.ToString()));
    }
    
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}
```

### Step-by-step verification
1. **Test input mapping**: Press LMB/RMB/1-4 and verify on-screen messages show correct InputTag values
2. **Check ASC connectivity**: Add logs in ASC methods to confirm calls are reaching the ability system  
3. **Verify tag matching**: Use breakpoints to inspect Spec.GetDynamicSpecSourceTags() in ASC
4. **Test ability activation**: Confirm abilities with matching StartupInputTag activate when inputs are pressed

### Common verification points
- InputConfig asset is assigned and populated in ATDPlayerController Blueprint
- GASInputMappingContext is assigned and added in BeginPlay
- Enhanced Input actions exist and are mapped to correct keys
- Ability StartupInputTags match the exact values in InputConfig
- Default Input Component Class is set to UTDEnhancedInputComponent

## Troubleshooting
- Default Input Component Class must be UTDEnhancedInputComponent
- GASInputMappingContext must be assigned and added in BeginPlay
- MoveAction and InputConfig must be set in BP/defaults
- Tags must match centralized names from FTDGameplayTags (InputTag.LMB/RMB/QuickSlot1..4)
- Guard null ASC in early frame calls