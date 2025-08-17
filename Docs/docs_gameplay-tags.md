# Gameplay Tags

## What and Why

- Hierarchical FNames (e.g., "Effect.Potion.Heal" or "Cue.Fire.Burn")
- Stored in `FGameplayTagContainer` with tag map counts
- Used pervasively in GAS to gate abilities, annotate effects, drive cues, and filter logic

## Effect Asset Tags to UI

- Effects can carry Asset Tags
- When applied, capture the effect’s asset tags and route to UI

### Recommended Pattern

In ASC subclass:
- Bind to `OnGameplayEffectAppliedDelegateToSelf`
- Extract tag container (`Spec.Def->InheritableOwnedTagsContainer` and/or asset tags on spec)
- Broadcast via ASC-level delegate `FEffectAssetTags` to interested systems

In Widget Controller:
- Bind to ASC’s `FEffectAssetTags` (use a lambda for concision)
- Iterate tags and react (log/show toasts/drive icons)

## Matching and Filtering

- Exact tag match or “matches any parent” logic
- Use GameplayTagAssetInterface on assets if needed