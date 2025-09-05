# GASCore Projectile Actors

Last updated: 2024-12-19

## Overview

The GASCore plugin provides a foundational projectile actor system designed to integrate seamlessly with the Gameplay Ability System (GAS). The projectile actors follow a compositional design pattern allowing for flexible customization while maintaining performance and replication reliability.

## Core Classes

### AGASCoreProjectileActor

The base projectile actor class providing essential components and collision handling for all projectile types.

**Key Features:**
- Modular component architecture (collision, movement, visuals)
- Collision event handling with overlap detection
- Blueprint-extensible design for visual and gameplay customization
- Performance-optimized with disabled tick by default

**Components:**

| Component | Type | Purpose |
|-----------|------|---------|
| `DefaultSceneRoot` | `USceneComponent` | Neutral root component for designer attachment |
| `SphereCollision` | `USphereComponent` | Primary collision detection for projectile overlap |
| `ProjectileMovementComponent` | `UProjectileMovementComponent` | Physics-based movement with configurable parameters |

**Default Configuration:**
```cpp
// Movement settings
ProjectileMovementComponent->InitialSpeed = 550.f;
ProjectileMovementComponent->MaxSpeed = 550.f;
ProjectileMovementComponent->ProjectileGravityScale = 0.f;

// Collision responses
SphereCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
SphereCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
```

### Virtual Collision Handler

```cpp
virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult);
```

**Purpose:** Override point for collision behavior. Base implementation is empty, allowing derived classes to implement specific hit effects, damage application, or destruction logic.

## Project Integration

### ATDProjectileActor

Game-specific projectile implementation inheriting from `AGASCoreProjectileActor`:

```cpp
UCLASS()
class RPG_TOPDOWN_API ATDProjectileActor : public AGASCoreProjectileActor
{
    GENERATED_BODY()

public:
    ATDProjectileActor();

protected:
    virtual void BeginPlay() override;
    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult) override;
};
```

**Design Pattern:** `ATDProjectileActor` serves as the project-specific base class where common game mechanics (damage application, effect spawning, target validation) are implemented.

## Architecture Benefits

### Compositional Design
- **Separation of Concerns:** Movement, collision, and visuals are handled by separate components
- **Designer Flexibility:** Visual effects, additional collision shapes, and audio can be added in Blueprint
- **Performance:** Minimal overhead with disabled tick and optimized collision queries

### GAS Integration Points
- **Spawning:** Projectiles typically spawned by `UGameplayAbility` subclasses
- **Effects:** Can apply `GameplayEffects` on collision via ability context
- **Targeting:** Integrates with ability targeting systems for predictive trajectories

### Replication Considerations
- **Movement Replication:** Handled automatically by `UProjectileMovementComponent`
- **Collision Authority:** Overlap events processed on server with results replicated
- **Visual Effects:** Local cosmetic effects can be triggered on collision events

## Usage Patterns

### Spawning from Abilities

```cpp
// In a projectile ability class
void UProjectileAbility::SpawnProjectile(const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!HasAuthority()) return;

    FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
    FRotator SpawnRotation = Direction.Rotation();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetAvatarActorFromActorInfo();
    SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    GetWorld()->SpawnActor<ATDProjectileActor>(
        ProjectileClass, 
        StartLocation, 
        SpawnRotation, 
        SpawnParams
    );
}
```

### Collision-based Effect Application

```cpp
// In ATDProjectileActor derived class
void ATDProjectileActor::OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereCollisionOverlap(OverlappedComponent, OtherActor, OtherComp, 
        OtherBodyIndex, bFromSweep, SweepResult);

    // Validate target
    if (!IsValidTarget(OtherActor)) return;
    
    // Apply gameplay effect through ASC
    if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
    {
        if (DamageEffectClass)
        {
            FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
            EffectContext.AddSourceObject(this);
            
            FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(
                DamageEffectClass, 1.0f, EffectContext);
                
            if (EffectSpec.IsValid())
            {
                TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
            }
        }
    }
    
    // Trigger destruction/impact effects
    HandleImpact(SweepResult);
}
```

## Blueprint Extension Points

### Visual Customization
1. **Mesh/Effects:** Attach `UStaticMeshComponent` or `UNiagaraComponent` to `DefaultSceneRoot`
2. **Trail Effects:** Add particle systems for projectile trails
3. **Impact Effects:** Override `OnSphereCollisionOverlap` in Blueprint for impact VFX

### Collision Customization
1. **Additional Collision:** Add extra collision components for complex shapes
2. **Collision Channels:** Configure custom collision responses for gameplay-specific interactions
3. **Trigger Volumes:** Add trigger components for proximity-based effects

## Performance Considerations

### Optimization Features
- **Disabled Tick:** Projectiles use physics simulation instead of per-frame updates
- **Minimal Collision:** Single sphere collision for broad-phase detection
- **Component Pooling:** Consider object pooling for frequently spawned projectiles

### Network Optimization
- **Prediction:** Client-side spawning with server reconciliation for smooth gameplay
- **Relevancy:** Projectiles automatically culled based on player relevance
- **Effect Batching:** Group multiple projectile effects for reduced network traffic

## Troubleshooting

### Common Issues

**Projectiles Not Moving:**
- Verify `ProjectileMovementComponent` has non-zero `InitialSpeed`
- Check spawn rotation is valid (not NaN or zero)
- Ensure projectile isn't colliding with spawner on creation

**Collision Not Detected:**
- Verify collision channels are properly configured
- Check target actors have collision components that respond to projectile
- Ensure `OnSphereCollisionOverlap` is properly bound in `BeginPlay`

**Network Desync:**
- Confirm projectiles only spawned on server (`HasAuthority()` check)
- Verify collision handling is server-authoritative
- Check effect application uses proper ASC context

**Performance Issues:**
- Profile projectile count in play sessions
- Implement projectile pooling for high-frequency spawning
- Consider lifetime limits for long-range projectiles

## See Also

- [Projectile Abilities Pattern](projectile-abilities.md)
- [Gameplay Effects](../gas/gameplay-effects.md)
- [Ability Targeting](../gas/abilities/targeting.md)
- [GAS Integration Guide](../gas/abilities/overview.md)
- [Performance Optimization](../architecture/performance.md)