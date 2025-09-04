# TDProjectileActor

Last updated: 2025-01-16

## Overview

`ATDProjectileActor` is the TopDown RPG-specific projectile actor that extends `AGASCoreProjectile` to provide game-specific collision handling and projectile behavior. This class serves as the foundation for all projectile-based abilities and attacks in the TopDown RPG system.

Since the base `AGASCoreProjectile` class is minimal, this implementation adds essential projectile functionality including collision detection, damage application, and GAS integration.

## Purpose and Responsibilities

The TDProjectileActor handles:

- **Collision Detection**: Sphere-based collision detection optimized for overhead perspective gameplay
- **Damage Application**: Integration with the Gameplay Ability System for applying damage and effects
- **Target Acquisition**: Automatic target detection and validation for projectile impacts
- **Visual Effects**: Coordination with visual and audio systems for impact feedback
- **Network Replication**: Proper authority handling for multiplayer gameplay

## Class Hierarchy

```
AActor
└── AGASCoreProjectile (GASCore Plugin - Basic Implementation)
    └── ATDProjectileActor (TopDown RPG - Enhanced with Collision & GAS Integration)
```

**Note**: The current `AGASCoreProjectile` class is a basic actor implementation. The `ATDProjectileActor` class extends it with collision detection, GAS integration, and projectile-specific functionality.

## Key Features

### Collision Handling

The projectile uses sphere collision detection for reliable target acquisition:

```cpp
virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult) override;
```

**Key Points:**
- Inherits base collision logic from `AGASCoreProjectileActor`
- Optimized for overhead/isometric perspective gameplay
- Supports both sweep and overlap-based collision detection
- Handles complex collision scenarios with multiple body indices

### Performance Optimization

```cpp
// Tick disabled by default for better performance
PrimaryActorTick.bCanEverTick = false;
```

The projectile is designed for efficiency:
- **No Tick by Default**: Tick is disabled unless specifically needed
- **Event-Driven**: Uses collision events rather than continuous polling
- **Lightweight**: Minimal overhead when spawned in large quantities

## Usage Patterns

### Basic Projectile Spawning

```cpp
// C++ - Spawn projectile from ability
FVector SpawnLocation = GetActorLocation();
FRotator SpawnRotation = GetActorRotation();

ATDProjectileActor* Projectile = GetWorld()->SpawnActor<ATDProjectileActor>(
    TDProjectileClass, SpawnLocation, SpawnRotation);
    
if (Projectile)
{
    // Configure projectile properties
    Projectile->SetOwner(this);
    // Set additional projectile parameters
}
```

### Blueprint Integration

The class is fully Blueprint-compatible:

1. **Create Blueprint Child**: Inherit from `ATDProjectileActor` in Blueprint
2. **Configure Collision**: Set up sphere collision component properties
3. **Add Visual Effects**: Attach particle systems and audio components
4. **Override Events**: Implement custom collision handling if needed

### Ability System Integration

```cpp
// In a GameplayAbility
UCLASS()
class UTDProjectileAbility : public UTDGameplayAbilityBase
{
public:
    virtual void ActivateAbility(...) override
    {
        // Spawn projectile with ability context
        ATDProjectileActor* Projectile = SpawnProjectile();
        
        // Configure with ability-specific data
        SetupProjectileEffects(Projectile);
    }
    
private:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<ATDProjectileActor> ProjectileClass;
};
```

## Configuration

### Collision Setup

Configure the sphere collision component for optimal performance:

```cpp
// In constructor or BeginPlay
USphereComponent* SphereCollision = GetSphereComponent();
if (SphereCollision)
{
    SphereCollision->SetCollisionRadius(50.0f);
    SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}
```

### Movement Configuration

Projectile movement is typically handled by the base class, but can be customized:

```cpp
// Configure projectile movement (in Blueprint or C++)
UProjectileMovementComponent* Movement = GetProjectileMovementComponent();
if (Movement)
{
    Movement->InitialSpeed = 1000.0f;
    Movement->MaxSpeed = 1000.0f;
    Movement->bRotationFollowsVelocity = true;
    Movement->bShouldBounce = false;
}
```

## Common Use Cases

### 1. Fireball Projectile

A basic magical projectile that applies fire damage:

```cpp
UCLASS(BlueprintType)
class ATDFireballProjectile : public ATDProjectileActor
{
    GENERATED_BODY()
    
public:
    ATDFireballProjectile()
    {
        // Configure for fire damage
        DamageType = EDamageType::Fire;
        BaseDamage = 50.0f;
    }
    
protected:
    virtual void OnSphereCollisionOverlap(...) override
    {
        // Apply fire effect on impact
        ApplyFireEffect(OtherActor);
        
        // Call parent implementation
        Super::OnSphereCollisionOverlap(...);
        
        // Destroy projectile after impact
        Destroy();
    }
};
```

### 2. Ice Shard Projectile

A projectile that applies slowing effects:

```cpp
UCLASS(BlueprintType)
class ATDIceShardProjectile : public ATDProjectileActor
{
    GENERATED_BODY()
    
protected:
    virtual void OnSphereCollisionOverlap(...) override
    {
        if (IsValidTarget(OtherActor))
        {
            // Apply slow effect through GAS
            ApplySlowEffect(OtherActor);
        }
        
        Super::OnSphereCollisionOverlap(...);
    }
    
private:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> SlowEffectClass;
};
```

### 3. Piercing Arrow

A projectile that can hit multiple targets:

```cpp
UCLASS(BlueprintType)
class ATDPiercingArrowProjectile : public ATDProjectileActor
{
    GENERATED_BODY()
    
protected:
    virtual void OnSphereCollisionOverlap(...) override
    {
        // Check if we've already hit this target
        if (!HitTargets.Contains(OtherActor))
        {
            HitTargets.Add(OtherActor);
            
            // Apply damage but don't destroy
            ApplyDamage(OtherActor);
            
            // Check if we've hit max targets
            if (HitTargets.Num() >= MaxTargets)
            {
                Destroy();
            }
        }
    }
    
private:
    UPROPERTY(EditDefaultsOnly)
    int32 MaxTargets = 3;
    
    UPROPERTY()
    TArray<AActor*> HitTargets;
};
```

## Integration with GASCore

The TDProjectileActor extends the basic `AGASCoreProjectile` class with enhanced functionality:

### Current Implementation Notes

The current `AGASCoreProjectile` base class provides minimal functionality (basic Actor lifecycle). The `ATDProjectileActor` implementation adds:

- Collision detection and handling
- GAS integration for damage and effects
- Projectile-specific movement and behavior
- Target validation and hit detection

### Future Enhancements

Consider enhancing the `AGASCoreProjectile` base class in the GASCore plugin to include:
- Built-in collision components
- GAS effect application methods
- Standard projectile movement components
- Networking optimization

### Effect Application

```cpp
// To be implemented with enhanced GASCore integration
virtual void ApplyGameplayEffectToTarget(AActor* Target, 
    TSubclassOf<UGameplayEffect> EffectClass);
```

### Damage System Integration

```cpp
// Damage is applied through the Gameplay Ability System
void ATDProjectileActor::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
    if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
    {
        // Create damage effect spec
        FGameplayEffectSpecHandle DamageSpec = CreateDamageEffect(DamageAmount);
        
        // Apply damage through GAS
        TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
    }
}
```

## Blueprint Events

The class supports several Blueprint events for customization:

- **OnProjectileHit**: Called when the projectile hits a valid target
- **OnProjectileDestroyed**: Called when the projectile is destroyed
- **OnEffectApplied**: Called when a gameplay effect is applied to a target

## Performance Considerations

### Pooling

For high-frequency projectiles, consider object pooling:

```cpp
// Project-specific projectile manager
class UTDProjectilePool : public UObject
{
public:
    ATDProjectileActor* GetPooledProjectile(TSubclassOf<ATDProjectileActor> ProjectileClass);
    void ReturnProjectileToPool(ATDProjectileActor* Projectile);
    
private:
    TMap<TSubclassOf<ATDProjectileActor>, TArray<ATDProjectileActor*>> ProjectilePools;
};
```

### Collision Optimization

- Use appropriate collision channels for projectile vs. target detection
- Configure collision responses to ignore unnecessary objects
- Consider using line traces for very fast projectiles

### Network Optimization

- Projectiles are typically spawned on the server and replicated to clients
- Consider client-side prediction for responsive feel
- Use appropriate network relevancy settings

## Debugging and Testing

### Debug Visualization

Enable collision visualization during development:

```cpp
#if !UE_BUILD_SHIPPING
void ATDProjectileActor::ShowDebugCollision()
{
    if (USphereComponent* Sphere = GetSphereComponent())
    {
        Sphere->SetHiddenInGame(false);
        Sphere->SetVisibility(true);
    }
}
#endif
```

### Console Commands

Useful console commands for debugging:

```
showdebug collision      // Show collision shapes
stat game               // Show general game stats
showdebug abilitysystem // Show GAS-related debug info
```

### Testing Checklist

- [ ] Projectile spawns correctly from abilities
- [ ] Collision detection works with intended targets
- [ ] Effects apply correctly on impact
- [ ] Performance is acceptable with multiple projectiles
- [ ] Network replication behaves correctly
- [ ] Visual and audio effects trigger appropriately
- [ ] Projectile cleanup works properly (no memory leaks)

## Common Issues and Solutions

### Issue: Projectile Passes Through Targets

**Cause**: Collision detection not configured properly or projectile moving too fast.

**Solution**:
```cpp
// Enable continuous collision detection
UProjectileMovementComponent* Movement = GetProjectileMovementComponent();
if (Movement)
{
    Movement->bShouldBounce = false;
    Movement->ProjectileGravityScale = 0.0f;
    
    // For very fast projectiles
    Movement->bIsSliding = false;
}

// Ensure proper collision settings
USphereComponent* Collision = GetSphereComponent();
if (Collision)
{
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}
```

### Issue: Multiple Hit Detection

**Cause**: Collision event firing multiple times for the same target.

**Solution**:
```cpp
// Track hit targets to avoid multiple hits
UPROPERTY()
TArray<AActor*> AlreadyHitTargets;

virtual void OnSphereCollisionOverlap(...) override
{
    if (AlreadyHitTargets.Contains(OtherActor))
    {
        return; // Already hit this target
    }
    
    AlreadyHitTargets.Add(OtherActor);
    Super::OnSphereCollisionOverlap(...);
}
```

### Issue: Performance Problems with Many Projectiles

**Cause**: Too many projectiles active simultaneously.

**Solution**:
```cpp
// Implement projectile pooling
// Set maximum active projectile limits
// Use simpler collision shapes for basic projectiles
// Consider using line traces for very fast projectiles
```

## Advanced Techniques

### Predictive Targeting

For AI or assisted targeting:

```cpp
FVector CalculateInterceptPoint(AActor* Target, float ProjectileSpeed)
{
    FVector TargetVelocity = Target->GetVelocity();
    FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
    
    // Calculate interception point
    float TimeToIntercept = ToTarget.Size() / ProjectileSpeed;
    return Target->GetActorLocation() + (TargetVelocity * TimeToIntercept);
}
```

### Homing Projectiles

```cpp
UCLASS()
class ATDHomingProjectile : public ATDProjectileActor
{
    GENERATED_BODY()
    
public:
    virtual void Tick(float DeltaTime) override
    {
        if (TargetActor && IsValid(TargetActor))
        {
            FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            GetProjectileMovementComponent()->Velocity = Direction * GetProjectileMovementComponent()->InitialSpeed;
        }
        
        Super::Tick(DeltaTime);
    }
    
    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor;
};
```

## See Also

- [GASCore Projectile Documentation](../../plugin/gascore-projectile.md)
- [Gameplay Abilities Overview](../../gas/abilities/overview.md)
- [Collision System Guide](../../architecture/collision-system.md)
- [Performance Optimization](../../guides/performance-optimization.md)
- [Multiplayer Considerations](../../architecture/replication-and-multiplayer.md)