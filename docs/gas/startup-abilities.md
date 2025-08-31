# Startup Abilities System

## Overview

The startup abilities system allows characters to automatically receive and grant gameplay abilities during initialization. This provides a server-authoritative pipeline for giving abilities to both player and AI characters.

## Components

### TDGameplayAbility (Base Class)
- Base C++ class derived from `UGameplayAbility`
- Provides project-wide defaults and can be extended for specific abilities
- Located in: `Source/RPG_TopDown/Public/AbilitySystem/Abilities/TDGameplayAbility.h`

### StartupAbilities Array
- Exposed on `ATDCharacterBase` as an editable array
- Category: "GAS|Abilities"
- Type: `TArray<TSubclassOf<UTDGameplayAbility>>`
- Can be configured per character class in Blueprint or C++

## Usage

### In Blueprint
1. Create a Blueprint derived from `TDCharacterBase` (or `TDPlayerCharacter`/`TDEnemyCharacter`)
2. In the Details panel, find the "GAS|Abilities" section
3. Add ability classes to the "Startup Abilities" array
4. Optionally enable "Auto Test Activate First Ability" for testing

### In C++
```cpp
// In character constructor or setup
StartupAbilities.Add(UTDTestAbility::StaticClass());
// Add more ability classes as needed
```

## Testing Features

### Automatic Test Activation
- Set `bAutoTestActivateFirstAbility = true` to automatically test-activate the first granted ability
- Useful for debugging and validating the ability granting pipeline
- Only activates on the server (authority)

### Manual Test Activation
- Call `TestActivateFirstAbility()` in Blueprint or C++
- Can be used as a Button event in Editor for testing
- Will attempt to activate the first granted ability

## Implementation Details

- Abilities are granted on the server only (HasAuthority() check)
- Granting happens after `InitAbilityActorInfo()` and attribute initialization
- Works for both player characters (ASC on PlayerState) and AI characters (ASC on Character)
- Each ability is granted at level 1 with no specific input binding (INDEX_NONE)

## Example Test Ability

`UTDTestAbility` is provided as a concrete example:
- Simple ability that logs activation and immediately ends
- Can be used to validate the pipeline is working
- Located in: `Source/RPG_TopDown/Public/AbilitySystem/Abilities/TDTestAbility.h`

## Server Authority

The system respects server authority:
- Abilities are only granted on the server
- Test activation only works on the server
- Replication of ability specs is handled by UE's Ability System Component