# End-to-end setup: Input tags → ASC → Ability activation

Last updated: 2025-09-03

This guide takes you from a clean project state to activating abilities via input tags using GASCore + TD layers in this repo.

Audience: UE devs familiar with C++/BP basics; following project's codebase conventions.

Outcome: Pressing LMB/RMB/1–4 fires callbacks in ATDPlayerController, routes to ASC, and activates abilities mapped to matching InputTag.*.

## Prerequisites
- Unreal Engine with these plugins enabled: GameplayAbilities, GameplayTags, Enhanced Input
- Project compiles and runs

## High-level architecture
- Centralized tags in FTDGameplayTags (InputTag.*, Attributes.*)
- Data-driven input config: UTDInputConfig (UGASCoreAbilityInputConfig)
- Binding component: UTDEnhancedInputComponent (UGASCoreEnhancedInputComponent)
- PlayerController: ATDPlayerController calls BindAbilityInputActions and forwards Held/Released
- ASC: UGASCoreAbilitySystemComponent responds to AbilityInputTagHeld/Released
- Abilities: UTDGameplayAbility (UGASCoreGameplayAbility) with StartupInputTag
- Init: UTDAbilityInitComponent grants StartupAbilities on authority

## Step 1 — Centralize and initialize gameplay tags
1) Ensure FTDGameplayTags and TDGameplayTags2 registration file exist (they do in this repo).
2) Call FTDGameplayTags::InitializeNativeGameplayTags() once at startup:
   - Preferred: Your game module StartupModule()
   - Alternative: GameInstance::Init()
3) Verify in Editor: Project Settings → Gameplay Tags shows InputTag.* and Attributes.*

Snippet (module):
```cpp
void FRPG_TopdownModule::StartupModule()
{
    FTDGameplayTags::InitializeNativeGameplayTags();
}
```
Access tags:
```cpp
const FTDGameplayTags& Tags = FTDGameplayTags::Get();
FGameplayTag LMB = Tags.InputTag_LMB; // "InputTag.LMB"
```

## Step 2 — Create Enhanced Input assets
1) Create UInputAction assets for: LMB, RMB, QuickSlot1..4 (Axis1D float is sufficient)
2) Create/extend UInputMappingContext and map actions:
   - IA_LMB → Mouse Left Button
   - IA_RMB → Mouse Right Button
   - IA_QuickSlot1..4 → Keyboard 1..4
3) Save assets (e.g., Content/Input/)

## Step 3 — Create a data-driven input config
1) Create a UTDInputConfig asset (data asset)
2) Add entries (InputAction, InputTag):
   - IA_LMB ↔ FTDGameplayTags::Get().InputTag_LMB
   - IA_RMB ↔ FTDGameplayTags::Get().InputTag_RMB
   - IA_QuickSlot1..4 ↔ corresponding QuickSlot tags
Notes:
- FindAbilityInputActionByTag uses MatchesTag (hierarchy-friendly)
- Keep tags exact under InputTag.* to align with ASC's HasTagExact matching

## Step 4 — Set default input component and assign assets
1) Project Settings → Input → Default Input Component Class = UTDEnhancedInputComponent
2) In ATDPlayerController Blueprint/Defaults, assign:
   - GASInputMappingContext (your IMC)
   - MoveAction (if used)
   - InputConfig (your UTDInputConfig)

## Step 5 — Ensure mapping context is added at runtime
In ATDPlayerController::BeginPlay:
```cpp
if (const ULocalPlayer* LP = Cast<ULocalPlayer>(GetLocalPlayer()))
{
    if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        EISubsystem->AddMappingContext(GASInputMappingContext, 0);
    }
}
```
Also configure mouse/UI input mode as needed for top-down control.

## Step 6 — Bind ability inputs
In ATDPlayerController::SetupInputComponent:
```cpp
UTDEnhancedInputComponent* EIC = CastChecked<UTDEnhancedInputComponent>(InputComponent);

if (InputConfig)
{
    EIC->BindAbilityInputActions(InputConfig, this, 
                                 &ThisClass::AbilityInputActionTagPressed,
                                 &ThisClass::AbilityInputActionReleased,
                                 &ThisClass::AbilityInputActionTagHeld);
}
```
Callbacks (header needs GameplayTagContainer.h):
```cpp
void AbilityInputActionTagPressed(FGameplayTag InputTag);
void AbilityInputActionReleased(FGameplayTag InputTag);
void AbilityInputActionHeld(FGameplayTag InputTag);
```

## Step 7 — Cache and forward to the ASC
```cpp
UTDAbilitySystemComponent* ATDPlayerController::GetASC()
{
    if (!TDAbilitySystemComponent)
    {
        TDAbilitySystemComponent = Cast<UTDAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
    }
    return TDAbilitySystemComponent;
}

void ATDPlayerController::AbilityInputActionHeld(FGameplayTag InputTag)
{
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}

void ATDPlayerController::AbilityInputActionReleased(FGameplayTag InputTag)
{
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagReleased(InputTag);
    }
}
```

## Step 8 — Grant abilities with StartupInputTag
1) Base ability class: UGASCoreGameplayAbility has StartupInputTag (exposed on CDO/BP)
2) Set StartupInputTag on ability BPs using FTDGameplayTags (e.g., InputTag_LMB)
3) Ensure your pawn/character has UTDAbilityInitComponent (or base UGASCoreAbilityInitComponent) and that StartupAbilities includes your ability classes
4) On authority, AddCharacterAbilities() will:
   - Create FGameplayAbilitySpec
   - Add GA->StartupInputTag to Spec.GetDynamicSpecSourceTags()
   - GiveAbility(Spec)

## Step 9 — Activation semantics inside ASC
UGASCoreAbilitySystemComponent::AbilityInputTagHeld:
- if (!InputTag.IsValid()) return
- Loop GetActivatableAbilities()
- For specs where Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag):
  - AbilitySpecInputPressed(Spec)   // set input flag and forward to instances
  - if (!Spec.IsActive()) TryActivateAbility(Spec.Handle)
Released:
- Same loop, call AbilitySpecInputReleased(Spec)

## Step 10 — Verify end-to-end
- Use on-screen debug in controller handlers to print InputTag
- Put breakpoints in ASC methods and inspect Spec.GetDynamicSpecSourceTags()
- Press LMB → ability with StartupInputTag=LMB should activate if not on cooldown and passes cost checks

## Notes on networking/authority
- Ability grants happen on authority (server). UTDAbilityInitComponent checks HasAuthority.
- Ability input forwarding from ATDPlayerController runs on the owning client. GAS supports client-predicted activation; server validates. Ensure Ability's Net Execution/Prediction policies match your design.
- If testing dedicated server, confirm controller exists on client and ASC is replicated/initialized (InitAbilityActorInfo) before input events.

## Common pitfalls
- Default Input Component Class not set to UTDEnhancedInputComponent → BindAbilityInputActions never runs
- InputConfig or IMC not assigned → no callbacks
- StartupInputTag not set on ability CDO → spec lacks InputTag → no match
- Using parent tags or typos (InputTag.QuickSlot-1) → HasTagExact will not match
- Forgetting to call InitializeNativeGameplayTags → tags default to empty