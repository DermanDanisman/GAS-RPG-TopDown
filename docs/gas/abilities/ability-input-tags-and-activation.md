# Ability Input Tags and Activation Flow (ASC-centric)

Last updated: 2025-09-02

This guide covers how abilities are matched and activated by input tag, based on the GASCore + TD patterns in this project.

## Quick-reference diagram

(See sequence/flowchart visuals at the top of this file; labels now match: UTDEnhancedInputComponent, PlayerController, ASC, TryActivateAbility.)

## Startup Input Tag on Gameplay Abilities

- Base class: UGASCoreGameplayAbility
- Add UPROPERTY(EditDefaultsOnly, Category=Input) FGameplayTag StartupInputTag
- Configure on the CDO (class defaults) or derived BP. Used for initial mapping when granting startup abilities.

## Add StartupInputTag to the AbilitySpec (dynamic source tags)

When granting startup abilities (authority) in your Ability System Component or init component:
- Create FGameplayAbilitySpec for the ability.
- If the ability is UGASCoreGameplayAbility and StartupInputTag is valid, add it to Spec.GetDynamicSpecSourceTags().
- GiveAbility(Spec) to register the ability.

Code sketch (from GASCore):

```cpp
for (const TSubclassOf<UGameplayAbility>& StartupAbility : StartupAbilities)
{
    FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(StartupAbility, 1);
    if (const UGASCoreGameplayAbility* GASCoreAbility = Cast<UGASCoreGameplayAbility>(AbilitySpec.Ability))
    {
        if (GASCoreAbility->StartupInputTag.IsValid())
        {
            AbilitySpec.GetDynamicSpecSourceTags().AddTag(GASCoreAbility->StartupInputTag);
        }
    }
    GiveAbility(AbilitySpec);
}
```

Why dynamic source tags?
- They're intended for runtime add/remove, enabling remapping (e.g., move an ability from LMB to RMB) without touching the ability class.

## Handling input in the ASC

Implement two methods on your ASC and call them from your PlayerController's Held/Released callbacks:

- AbilityInputTagHeld(const FGameplayTag& InputTag)
- AbilityInputTagReleased(const FGameplayTag& InputTag)

Held flow (aligned to code):
- if (!InputTag.IsValid()) return
- Iterate GetActivatableAbilities()
- For each Spec where Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag):
  - AbilitySpecInputPressed(Spec)
  - If not active: TryActivateAbility(Spec.Handle)

Released flow:
- if (!InputTag.IsValid()) return
- Iterate GetActivatableAbilities()
- For each Spec where Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag):
  - AbilitySpecInputReleased(Spec)

Notes:
- TryActivateAbility respects costs/cooldowns and may fail; that's expected.
- AbilitySpecInputPressed/Released toggles an internal input flag and forwards to instances (override InputPressed/InputReleased in your abilities if needed).
- Do not forcibly end on release unless that's your design; let abilities decide.

## PlayerController integration recap (TD layer)

- ATDPlayerController handlers: AbilityInputActionTagPressed/Released/Held(FGameplayTag)
- Cache ASC: UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()) cast to UTDAbilitySystemComponent
- Forward Held/Released to ASC; optional behavior on Pressed

## Ability initialization (authority-only)

- Components: UGASCoreAbilityInitComponent (base), UTDAbilityInitComponent (game layer)
- At startup on authority, AddCharacterAbilities() collects StartupAbilities and calls ASC->AddCharacterAbilities()
- ASC adds UGASCoreGameplayAbility::StartupInputTag to Spec.GetDynamicSpecSourceTags() then GiveAbility(Spec)

## Tag matching semantics (important)

- In ASC input handling, we use HasTagExact on Spec.GetDynamicSpecSourceTags() for precise matching.
- In Input Config lookups (FindAbilityInputActionByTag), UGASCoreAbilityInputConfig uses MatchesTag to allow parent/child relationships in your tag hierarchy.

## Debugging tips
- Use on-screen debug to confirm tags reaching the controller
- Break in ASC and inspect ActivatableAbilities; verify StartupInputTag is present in Spec.GetDynamicSpecSourceTags()
- If abilities re-activate every frame, confirm they aren't ending immediately; otherwise that's expected with a Held loop

## Runtime remapping
- Remove the current InputTag from a Spec's DynamicSpecSourceTags and add a different InputTag; Held/Released routing follows the new tag automatically