# GASCore Plugin Documentation Index

Last updated: 2025-01-21

## Overview

The GASCore plugin provides a comprehensive framework for implementing gameplay abilities, projectiles, and related systems in Unreal Engine using the Gameplay Ability System (GAS).

## Core Documentation

### System Overview
- [Plugin Overview](overview.md) - High-level design and benefits
- [API Reference](api.md) - Complete class and function reference
- [Combat Interface](combat-interface.md) - Actor integration patterns

### Gameplay Abilities
- [Projectile Abilities Pattern](projectile-abilities.md) - Complete projectile ability implementation guide
- [Projectile Actors](projectile-actors.md) - Actor spawning and management patterns

## Ability Tasks

Ability tasks provide asynchronous functionality for complex gameplay patterns:

- [Gameplay Ability Tasks](gameplay-ability-tasks.md) - Core concepts and base task implementation
- [Ability Task Montage Integration](ability-task-montage.md) - Animation-driven ability patterns
- [GASCore Aim Trace Task](gascore-aim-trace-task.md) - Target selection through line tracing

## Actor Systems

- [Spawn Projectile System](spawn-projectile.md) - Comprehensive projectile spawning framework
- [GASCore Spawned Actor](gascore-spawned-actor.md) - Base class for ability-spawned actors

## Integration Guides

- [Projectile Implementation Guide](../guides/projectile-implementation-guide.md) - Step-by-step implementation
- [Gameplay Abilities Concepts and Practice](../guides/GameplayAbilities_Concepts_And_Practice.md) - Deep dive into GAS concepts
- [Instructor Implementation Deep Dive](../guides/Instructor_Implementation_DeepDive.md) - Advanced patterns and techniques

## Quick Reference

### Class Hierarchy
```
UGameplayAbility
└─ UGASCoreGameplayAbility
   └─ UGASCoreProjectileAbility

UAbilityTask
└─ UGASCoreTargetDataFromAimTrace

AActor
└─ AGASCoreSpawnedActorByGameplayAbility
```

### Key Interfaces
- `IGASCoreCombatInterface` - Actor combat integration
- `IAbilitySystemInterface` - Standard GAS interface

### Essential Workflow
1. **Create Ability Class** - Derive from `UGASCoreGameplayAbility` or `UGASCoreProjectileAbility`
2. **Configure Spawned Actor** - Set up `AGASCoreSpawnedActorByGameplayAbility` for projectiles
3. **Implement Combat Interface** - Add `IGASCoreCombatInterface` to characters
4. **Add Ability Tasks** - Use tasks like `UGASCoreTargetDataFromAimTrace` for complex behavior
5. **Set Up Effects** - Configure damage and other gameplay effects

## Getting Started

1. Read [Plugin Overview](overview.md) for system understanding
2. Follow [Projectile Implementation Guide](../guides/projectile-implementation-guide.md) for your first ability
3. Study [Gameplay Ability Tasks](gameplay-ability-tasks.md) for advanced patterns
4. Reference [API Documentation](api.md) for specific function details

## Common Patterns

### Basic Projectile Ability
```cpp
void UMyProjectileAbility::ActivateAbility(/*...*/)
{
    if (!CommitAbility(/*...*/)) return;
    SpawnActorFromGameplayAbility();
    K2_EndAbility();
}
```

### Targeted Projectile with Aim Trace
```cpp
void UMyTargetedAbility::ActivateAbility(/*...*/)
{
    UGASCoreTargetDataFromAimTrace* AimTask = 
        UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(this);
    AimTask->ValidHitResultData.AddDynamic(this, &UMyTargetedAbility::OnTargetSelected);
    AimTask->ReadyForActivation();
}
```

### Montage-Driven Ability
```cpp
void UMyMeleeAbility::ActivateAbility(/*...*/)
{
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::PlayMontageAndWait(this, AttackMontage);
    MontageTask->OnCompleted.AddDynamic(this, &UMyMeleeAbility::OnMontageComplete);
    MontageTask->ReadyForActivation();
}
```

## Troubleshooting

### Common Issues
- **Projectiles not spawning**: Check server authority and combat interface implementation
- **Abilities not activating**: Verify ability granting and input binding
- **Tasks not completing**: Ensure proper delegate binding and task activation
- **Network issues**: Review replication settings and authority checks

### Debug Tools
- Console command: `showdebug abilitysystem`
- Debug CVars: `gas.debug.projectiles`, `gas.debug.aimtrace`, `gas.debug.spawnedactors`
- Logging: Enable `LogAbilitySystem` category

## See Also

- [Unreal Engine GAS Documentation](https://docs.unrealengine.com/5.3/en-US/gameplay-ability-system-for-unreal-engine/)
- [GAS Community Documentation](https://github.com/tranek/GASDocumentation)
- [Unreal Engine Projectile Movement](https://docs.unrealengine.com/5.3/en-US/projectile-movement-component-in-unreal-engine/)