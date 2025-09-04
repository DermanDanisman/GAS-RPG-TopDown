# Projectiles in GAS Abilities

Last updated: 2024-12-19

## Overview

Projectiles are actors spawned by abilities that travel through the world to deliver effects at target locations. In the context of GAS (Gameplay Ability System), projectiles serve as the delivery mechanism for spells, ranged attacks, and other distance-based abilities.

## Key Concepts

### Projectile Actor Base Class

A typical projectile implementation extends `AActor` and includes:
- **Movement Component**: For physics-based or custom movement
- **Collision Component**: For detecting hits with targets or environment
- **Mesh/Visual Component**: For visual representation
- **Gameplay Effect Application**: For applying damage/effects on impact

### Integration with Abilities

Projectiles are spawned during ability execution and carry information about:
- **Source Actor**: Who cast the spell/fired the projectile
- **Ability Context**: Level, modifiers, and other ability-specific data
- **Target Information**: Direction, target actor, or target location
- **Damage/Effect Data**: What gameplay effects to apply on hit

## Implementation Patterns

### Basic Projectile Actor

```cpp
// Header (.h)
UCLASS()
class MYGAME_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();

protected:
    virtual void BeginPlay() override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    class USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    class UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    class UStaticMeshComponent* ProjectileMesh;

    // Gameplay Effect to apply on hit
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    // Collision handling
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
    // Set damage effect class from spawning ability
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SetDamageEffectClass(TSubclassOf<UGameplayEffect> InDamageEffectClass);

    // Set damage effect params (e.g., damage amount via SetByCaller)
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SetDamageEffectParams(const FGameplayEffectSpecHandle& InDamageEffectSpecHandle);

protected:
    // Store effect spec for application on hit
    FGameplayEffectSpecHandle DamageEffectSpecHandle;
};
```

### Implementation (.cpp)

```cpp
AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true; // Enable replication for multiplayer

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetCollisionProfileName("Projectile");
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    // Create mesh component
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create movement component
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 1000.0f;
    ProjectileMovement->MaxSpeed = 1000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->Bounciness = 0.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f; // No gravity for magic projectiles
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind collision event
    CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
    
    // Set projectile lifetime
    SetLifeSpan(10.0f); // Destroy after 10 seconds if no hit
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Apply damage effect if we have a valid target with ASC
    if (OtherActor && OtherActor != GetInstigator())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor))
        {
            if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
            {
                if (DamageEffectSpecHandle.IsValid())
                {
                    // Apply the pre-configured damage effect
                    TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
                }
            }
        }
    }

    // Destroy projectile on hit
    Destroy();
}

void AProjectile::SetDamageEffectParams(const FGameplayEffectSpecHandle& InDamageEffectSpecHandle)
{
    DamageEffectSpecHandle = InDamageEffectSpecHandle;
}
```

### Spawning from Abilities

```cpp
// In your ability class
UCLASS()
class MYGAME_API UFireballAbility : public UTDGameplayAbilityBase
{
    GENERATED_BODY()

protected:
    // Projectile class to spawn
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TSubclassOf<AProjectile> ProjectileClass;

    // Damage effect to apply on hit
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    virtual bool ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    UFUNCTION(BlueprintCallable, Category = "Ability")
    void SpawnProjectile();
};

// Implementation
bool UFireballAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return false;
    }

    // Play casting animation, then spawn projectile
    // This is typically done through ability tasks like PlayMontageAndWait
    SpawnProjectile();

    return true;
}

void UFireballAbility::SpawnProjectile()
{
    if (!ProjectileClass || !HasAuthority(&CurrentActivationInfo))
    {
        return; // Only spawn on server
    }

    // Get spawn location and rotation
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        return;
    }

    FVector SpawnLocation = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * 100.0f;
    FRotator SpawnRotation = AvatarActor->GetActorRotation();

    // Spawn projectile
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = AvatarActor;
    SpawnParams.Instigator = Cast<APawn>(AvatarActor);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        // Create and configure damage effect
        if (DamageEffectClass)
        {
            FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
            FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
            
            // Configure damage via SetByCaller
            float DamageAmount = 50.0f; // Could be based on caster's attributes
            DamageEffectSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), DamageAmount);
            
            // Pass effect to projectile
            Projectile->SetDamageEffectParams(DamageEffectSpecHandle);
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

## Blueprint Implementation

### Projectile Setup in Blueprint

1. **Create Blueprint**: Inherit from your C++ projectile base class
2. **Configure Components**:
   - Set collision sphere radius
   - Assign mesh and materials
   - Configure projectile movement (speed, gravity, bounce)
3. **Set Damage Effect**: Assign the GameplayEffect class for damage
4. **Visual Effects**: Add particle systems, trails, or other VFX

### Ability Blueprint Implementation

```
Event ActivateAbility
 ↓
Check CommitAbility
 ↓ (Success)
PlayMontageAndWait (casting animation)
 ↓ (On Completed)
Spawn Projectile
 ↓
Set Damage Effect Params
 ↓
EndAbility
```

## Advanced Patterns

### Homing Projectiles

```cpp
// In projectile class
UPROPERTY(EditDefaultsOnly, Category = "Homing")
bool bIsHoming = false;

UPROPERTY(EditDefaultsOnly, Category = "Homing", meta = (EditCondition = "bIsHoming"))
float HomingAcceleration = 2000.0f;

UPROPERTY(BlueprintReadWrite, Category = "Homing", meta = (EditCondition = "bIsHoming"))
AActor* HomingTarget;

// In Tick or timer function
if (bIsHoming && HomingTarget)
{
    FVector Direction = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    ProjectileMovement->Velocity += Direction * HomingAcceleration * DeltaTime;
    ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetClampedToMaxSize(ProjectileMovement->MaxSpeed);
}
```

### Area of Effect Projectiles

```cpp
// On hit, apply effect to all targets in radius
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (HasAuthority())
    {
        // Get all actors in explosion radius
        TArray<AActor*> TargetsInRange;
        UKismetSystemLibrary::SphereOverlapActors(
            GetWorld(),
            GetActorLocation(),
            ExplosionRadius,
            TargetObjectTypes,
            TargetActorClass,
            ActorsToIgnore,
            TargetsInRange
        );

        // Apply damage effect to each target
        for (AActor* Target : TargetsInRange)
        {
            if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
            {
                if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
                {
                    if (DamageEffectSpecHandle.IsValid())
                    {
                        TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
                    }
                }
            }
        }
    }

    Destroy();
}
```

### Penetrating Projectiles

```cpp
// Track hit actors to avoid multi-hitting
UPROPERTY()
TArray<AActor*> HitActors;

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || HitActors.Contains(OtherActor))
    {
        return; // Already hit this actor
    }

    HitActors.Add(OtherActor);

    // Apply damage but don't destroy
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor))
    {
        // Apply damage logic...
    }

    // Check if we should stop (hit wall, max targets, etc.)
    bool bShouldStop = OtherActor->IsA<AStaticMeshActor>() || HitActors.Num() >= MaxTargets;
    if (bShouldStop)
    {
        Destroy();
    }
}
```

## Network Considerations

### Replication

```cpp
// In constructor
bReplicates = true;
bReplicateMovement = true; // For smooth movement on clients

// Replicated properties
UPROPERTY(Replicated)
float DamageAmount;

UPROPERTY(Replicated)
AActor* HomingTarget;

// GetLifetimeReplicatedProps
void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AProjectile, DamageAmount);
    DOREPLIFETIME(AProjectile, HomingTarget);
}
```

### Server Authority

- **Spawning**: Only spawn projectiles on the server
- **Damage Application**: Only apply gameplay effects on the server
- **Movement**: Let UE4's replication handle smooth movement
- **Destruction**: Server decides when to destroy

### Client Prediction

For responsive gameplay, consider:
- Immediate visual feedback on client (VFX, sound)
- Predictive projectile spawning for local player
- Server reconciliation for mispredicted hits

## Debugging and Testing

### Common Debug Commands

```cpp
// In projectile class
#if !UE_BUILD_SHIPPING
void AProjectile::OnHit(/* parameters */)
{
    UE_LOG(LogTemp, Warning, TEXT("Projectile hit %s at location %s"), 
        *OtherActor->GetName(), *Hit.Location.ToString());
    
    DrawDebugSphere(GetWorld(), Hit.Location, 50.0f, 12, FColor::Red, false, 5.0f);
}
#endif
```

### Console Variables

```cpp
// Add to your project's console variables
static TAutoConsoleVariable<bool> CVarDebugProjectiles(
    TEXT("game.DebugProjectiles"),
    false,
    TEXT("Show debug information for projectiles"),
    ECVF_Cheat
);
```

### Testing Checklist

- [ ] Projectile spawns at correct location and rotation
- [ ] Movement speed and trajectory work as expected
- [ ] Collision detection works with intended targets
- [ ] Damage effects apply correctly on hit
- [ ] Visual and audio effects trigger properly
- [ ] Network replication maintains projectile state
- [ ] Projectile cleanup (destruction/lifespan) works correctly
- [ ] Performance acceptable with multiple projectiles

## Common Pitfalls

### 1. **Collision Setup**
```cpp
// ❌ Wrong - May not detect hits properly
CollisionSphere->SetCollisionProfileName("IgnoreAll");

// ✅ Correct - Use appropriate collision profile
CollisionSphere->SetCollisionProfileName("Projectile");
```

### 2. **Authority Checks**
```cpp
// ❌ Wrong - Spawns on all clients
GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);

// ✅ Correct - Only spawn on server
if (HasAuthority(&CurrentActivationInfo))
{
    GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
}
```

### 3. **Damage Application**
```cpp
// ❌ Wrong - Direct attribute manipulation
TargetAttributeSet->SetHealth(TargetAttributeSet->GetHealth() - Damage);

// ✅ Correct - Use Gameplay Effects
TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
```

### 4. **Memory Leaks**
```cpp
// ❌ Wrong - Projectile may never be destroyed
// No lifespan or destruction logic

// ✅ Correct - Set reasonable lifespan
SetLifeSpan(10.0f);
```

## Performance Considerations

### Object Pooling

For high-frequency projectile spawning:

```cpp
// Create object pool manager
UCLASS()
class MYGAME_API UProjectilePool : public UObject
{
    GENERATED_BODY()

public:
    AProjectile* GetPooledProjectile();
    void ReturnProjectileToPool(AProjectile* Projectile);

private:
    UPROPERTY()
    TArray<AProjectile*> AvailableProjectiles;
    
    UPROPERTY()
    TArray<AProjectile*> ActiveProjectiles;
};
```

### LOD and Culling

```cpp
// Distance-based complexity reduction
float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerLocation);
if (DistanceToPlayer > MaxDetailDistance)
{
    // Reduce update frequency, disable some VFX, etc.
    SetActorTickInterval(0.1f); // Tick less frequently
}
```

## Integration Examples

### Fireball Spell

1. **Ability**: FireballAbility spawns projectile after casting animation
2. **Projectile**: Travels in straight line, applies fire damage on hit
3. **Effect**: Instant damage + burning DoT effect
4. **VFX**: Fire trail, explosion on impact, screen shake

### Magic Missile

1. **Ability**: Spawns multiple homing projectiles
2. **Projectile**: Each projectile homes to nearest enemy
3. **Effect**: Force damage, guaranteed hit
4. **VFX**: Glowing orbs with particle trails

### Grenade

1. **Ability**: Spawns projectile with arc trajectory
2. **Projectile**: Bounces once, then explodes after timer
3. **Effect**: AoE damage in explosion radius
4. **VFX**: Explosion effect, debris particles

## See Also

- [Gameplay Abilities Overview](overview.md)
- [Gameplay Effects](../gameplay-effects.md)
- [Ability Tasks and Async Operations](ability-tasks.md)
- [Replication and Multiplayer](../../architecture/replication-and-multiplayer.md)
- [Performance Optimization](../../guides/performance-optimization.md)