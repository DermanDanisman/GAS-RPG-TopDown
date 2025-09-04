# Actors Documentation

Last updated: 2025-01-16

This section documents the various actor classes in the TopDown RPG project, focusing on gameplay-specific implementations that extend the GASCore framework.

## Overview

The TopDown RPG project includes several specialized actor classes that provide game-specific functionality while leveraging the underlying GASCore plugin for Gameplay Ability System integration.

## Actor Classes

### Projectile Actors
- [TDProjectileActor](projectile/td-projectile-actor.md) - TopDown RPG-specific projectile implementation

### Gameplay Effect Actors
- [TDGameplayEffectActor](gameplay-effect/td-gameplay-effect-actor.md) - TopDown RPG-specific effect application actor

## Architecture Patterns

All TopDown RPG actors follow these architectural principles:

- **GAS Integration**: Inherit from GASCore base classes for consistent Ability System integration
- **Multiplayer Support**: Designed with proper replication and authority handling
- **Modular Design**: Component-based architecture for reusable functionality
- **Performance Optimization**: Efficient collision detection and event handling

## See Also

- [GASCore Plugin Documentation](../plugin/overview.md)
- [Gameplay Ability System](../gas/overview.md)
- [Architecture Overview](../architecture/overview.md)