# Gameplay Effects

Last updated: 2025-08-17

## Types

- Instant
  - One-off modifies Base; permanent (unless overwritten later)
- Has Duration
  - Applies temporary Current-value modification
  - Undone when duration ends
- Infinite
  - Applies temporary Current-value modification
  - Stays until explicitly removed

## Periodic

- Set `Period > 0` on Duration/Infinite effects
- Each tick applies an Instant-like permanent change (e.g., +1 Health per 0.1s)
- `ExecutePeriodicEffectOnApplication`:
  - If true, apply once immediately, then start ticking
  - If false, wait one full period before first tick

## Stacking

- Stacking Type:
  - None: multiple independent instances
  - Aggregate by Source: per-source stack limit
  - Aggregate by Target: per-target stack limit
- Policies:
  - Stack Limit Count (e.g., 1 or 2)
  - Duration Refresh (Never / Refresh on Success)
  - Period Reset (Never / Reset on Success)
  - Expiration (Clear Entire Stack / Remove Single Stack and Refresh / Refresh Duration)

## Effect Actor Pattern

Blueprint-friendly actor that applies effects on overlap:

- Data:
  - InstantGEClass (TSubclassOf<UGameplayEffect>)
  - DurationGEClass
  - InfiniteGEClass
  - Application Policy (ApplyOnOverlap / ApplyOnEndOverlap / DoNotApply) per type
  - Removal Policy (RemoveOnEndOverlap / DoNotRemove) for Infinite type
  - Optional: bDestroyOnEffectRemoval
- API:
  - `OnOverlap(AActor* TargetActor)`
  - `OnEndOverlap(AActor* TargetActor)`
  - Utility to apply spec (C++) or via Blueprint library nodes

Use cases:
- HP/MP potions (Instant)
- Health/Mana crystals (Duration + Periodic)
- Fire area (Infinite + Periodic, removed on end overlap)

## Blueprint vs C++

- Blueprint-only path:
  - `GetAbilitySystemComponent(Target)` (Blueprint Library)
  - `MakeEffectContext`, `MakeOutgoingSpec`, `ApplyGameplayEffectSpecToSelf`
- C++ path:
  - More control (e.g., AddSourceObject on context, custom removal, policies)