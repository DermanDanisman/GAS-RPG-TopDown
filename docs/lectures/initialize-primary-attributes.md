# Initialize Primary Attributes with a Gameplay Effect (GE)

Overview
- We previously initialized attributes via a DataTable on the ASC’s Default Starting Data. That was educational, but the preferred way in GAS is to use a Gameplay Effect (GE).
- This document explains the “why,” the minimal “how,” where to call it, and common pitfalls while following the tutorial pattern.

Why prefer a GE over a DataTable for init?
- Consistency: Uses the same pipeline as buffs/debuffs and other attribute changes.
- Extensibility: Easy to add SetByCaller magnitudes, level scaling, or saved-data restores later.
- Single source of truth: Avoids duplicate or competing initial values (no more “values show up twice”).

Editor setup (one-time)
1) Remove the DataTable from the ASC’s Default Starting Data (we won’t use it anymore).
2) Create an Init GE for your character:
   - Instant Gameplay Effect (applies once).
   - Add one Override modifier per primary attribute (e.g., Strength, Intelligence, Resilience, Vigor).
   - Set magnitudes to your initial values (e.g., STR=10, INT=17, RES=12, VIG=9).
3) Assign this GE asset to your character/owner in Blueprint (e.g., a “DefaultPrimaryAttributes” property on your init component or character).

Minimal code pattern (apply once)
- Place a TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes where you can set it from Blueprint (character or actor component).
- Apply after the ASC has valid ActorInfo (Owner and Avatar set).

Example (to Target, same ASC as target)
- Include headers in your .cpp: AbilitySystemComponent.h, GameplayEffect.h
- Example call:
```cpp
check(IsValid(AbilitySystemComponent));
check(DefaultPrimaryAttributes);

const FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultPrimaryAttributes, 1.f, Ctx);
AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent);

// Notes:
// - SpecHandle.Data is a shared pointer; use Data.Get() to get the pointer,
//   then dereference (*) because Apply expects a const ref.
```

Where to call it
- In InitAbilityActorInfo (after ASC->InitAbilityActorInfo(...)) is ideal: the ASC’s ActorInfo is valid at this point.
- Typical player flow:
  - Server: PossessedBy(...) -> InitializeAbilityActorInfo()
  - Client: OnRep_PlayerState() -> InitializeAbilityActorInfo()

Authority (server vs client)
- Recommended: Apply on the server only (HasAuthority()) and rely on attribute replication to update clients.
- Optional: You can apply on both server and client if the values are identical; the client won’t need to wait for replication.
- Practical tip: If InitializeAbilityActorInfo runs on both server and client, guard the init call with HasAuthority() to avoid double application.

Validation and checks
- Ensure the ASC is valid and DefaultPrimaryAttributes is set before applying (use check() during development or guards in production).
- Ensure InitAbilityActorInfo has been called (ASC ActorInfo must be valid). If you apply too early, defer until ready.

Testing
- In PIE, run: showdebug abilitysystem
- Verify primary attributes reflect the GE values (e.g., 10, 17, 12, 9).
- Confirm only one application occurred (avoid double-application).

Common compile/runtime issues and fixes
- “UGameplayEffect undeclared”: forward declare in headers or include the correct header where you use it.
- “ASC incomplete type”: include AbilitySystemComponent.h in the .cpp that calls Apply/MakeOutgoingSpec.
- Spec handle misuse: remember to dereference SpecHandle.Data.Get() when passing to Apply.
- Double initialization: calling from both server and client without guards can re-apply; use HasAuthority() at the call site.

Teachable moments (what to remember)
- GE for init keeps all attribute changes inside the same GAS flow you’ll use later for buffs/debuffs.
- Apply after ASC ActorInfo is valid to avoid race conditions.
- Authority matters: server-side application + replication is the simplest, most correct default.
- Override modifiers are clearer for initial values than Add, even if starting from zero.
- Keep the init GE asset in the project (game-specific values), not the plugin (the plugin provides the mechanism).