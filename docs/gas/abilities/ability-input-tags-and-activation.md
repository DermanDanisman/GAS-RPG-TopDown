# Ability Input Tags and Activation Flow (ASC-centric)

Last updated: 2024-12-19

This guide covers how abilities are matched and activated by input tag, based on the GASCore + TD patterns in this project.

## Quick-reference diagram (with legend)

Legend:
- Started = Pressed, Completed = Released, Triggered = Held (per-frame)
- InputTag family values come from FTDGameplayTags (InputTag.LMB/RMB/QuickSlot1..4)

(See sequence/flowchart visuals at the top of this file; labels now match: UTDEnhancedInputComponent, PlayerController, ASC, TryActivateAbility.)

## Using FTDGameplayTags for StartupInputTag

- Base ability: UGASCoreGameplayAbility has StartupInputTag
- Set StartupInputTag to values from FTDGameplayTags (e.g., FTDGameplayTags::Get().InputTag_LMB)

Checklist:
- [ ] Initialize native tags at startup
- [ ] Set ability CDO StartupInputTag in BP/Defaults
- [ ] Verify Spec.GetDynamicSpecSourceTags() contains the expected InputTag.* on grant

## Exact vs hierarchical matches

- ASC routing: HasTagExact on Spec.GetDynamicSpecSourceTags()
- Config lookup: MatchesTag in UTDInputConfig for flexibility

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

**Network authority considerations:**
- Ability granting must occur on the server (authority) to ensure proper replication
- UTDAbilityInitComponent checks HasAuthority() before granting abilities
- Client receives replicated ability specs but cannot grant new abilities
- InitAbilityActorInfo must be called on both server and client for proper setup

**Initialization sequence:**
1. Pawn spawned on server (HasAuthority() = true)
2. UTDAbilityInitComponent::BeginPlay() calls AddCharacterAbilities()  
3. For each ability in StartupAbilities array:
   - Create FGameplayAbilitySpec with ability class and level
   - If ability has valid StartupInputTag, add to Spec.GetDynamicSpecSourceTags()
   - Call ASC->GiveAbility(Spec) to register the ability
4. Specs replicate to client automatically
5. Client's ASC receives specs and can activate them via input

## Tag matching semantics (important)

- In ASC input handling, we use HasTagExact on Spec.GetDynamicSpecSourceTags() for precise matching.
- In Input Config lookups (FindAbilityInputActionByTag), UGASCoreAbilityInputConfig uses MatchesTag to allow parent/child relationships in your tag hierarchy.

## Debugging tips
- Use on-screen debug to confirm tags reaching the controller
- Break in ASC and inspect ActivatableAbilities; verify StartupInputTag is present in Spec.GetDynamicSpecSourceTags()
- If abilities re-activate every frame, confirm they aren't ending immediately; otherwise that's expected with a Held loop
- Enable Gameplay Debugger (apostrophe key) to inspect ASC state in real-time
- Check console for GAS-related warnings about missing abilities or tags
- Use UE_LOG(LogGameplayAbilities, Log, ...) for custom debugging output

## Networking and prediction notes

**Client prediction:**
- Input events trigger on owning client first (responsive feel)
- Server validates and may correct client predictions  
- Use NetExecutionPolicy and NetSecurityPolicy on abilities to control behavior
- InstantingPolicy affects when ability instances are created

**Common network policies:**
- LocalPredicted: Client starts immediately, server validates
- ServerInitiated: Server must approve before client can activate
- ServerOnly: Only server can activate (high security/authority)

**Replication considerations:**  
- Ability specs replicate automatically from server to client
- Dynamic source tags (including input tags) replicate with specs
- ASC state (active abilities, attributes) replicates continuously
- Input events are client-side and forward to server through GAS RPC system

**Multiplayer testing checklist:**
- Verify abilities grant on server (HasAuthority check)
- Confirm client receives replicated specs
- Test activation works on both client and dedicated server
- Check that input events reach ASC on owning client
- Validate server can override client predictions when necessary

## Runtime remapping
- Remove the current InputTag from a Spec's DynamicSpecSourceTags and add a different InputTag; Held/Released routing follows the new tag automatically
- See the dedicated runtime remapping guide for detailed examples and best practices