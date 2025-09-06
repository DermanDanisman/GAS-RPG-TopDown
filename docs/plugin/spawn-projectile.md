# Spawn Projectile System

Last updated: 2025-01-21

## Overview

The GASCore spawn projectile system provides a robust framework for creating projectile-based abilities in the Gameplay Ability System. It integrates spawned actors, ability tasks, and the combat interface to create consistent, replicable projectile mechanics.

This system is built around the `AGASCoreSpawnedActorByGameplayAbility` class and the `SpawnActorFromGameplayAbility()` method in `UGASCoreGameplayAbility`.

## Core Architecture

### Component Overview

```
UGASCoreGameplayAbility
    ├─ SpawnActorFromGameplayAbility()
    ├─ SpawnActorClass (TSubclassOf<AGASCoreSpawnedActorByGameplayAbility>)
    └─ IGASCoreCombatInterface (for spawn location)

AGASCoreSpawnedActorByGameplayAbility
    ├─ USceneComponent (DefaultSceneRoot)
    ├─ USphereComponent (SphereCollision)
    ├─ UProjectileMovementComponent (ProjectileMovementComponent)
    └─ OnSphereCollisionOverlap() (collision handling)
```

### Key Classes

#### UGASCoreGameplayAbility
Base class for all gameplay abilities with projectile spawning capability.

#### UGASCoreProjectileAbility
Specialized ability class for projectile-specific implementations.

#### AGASCoreSpawnedActorByGameplayAbility
Base projectile actor class with collision and movement components.

#### IGASCoreCombatInterface
Interface for determining spawn locations and combat-related data.

## Projectile Spawning Implementation

### Base Spawning Method

```cpp
void UGASCoreGameplayAbility::SpawnActorFromGameplayAbility()
{
    const FGameplayAbilityActivationInfo GameplayAbilityActivationInfo = GetCurrentActivationInfo();
    const bool bIsServer = HasAuthority(&GameplayAbilityActivationInfo);
    if (!bIsServer) return;

    if (!ensureAlwaysMsgf(SpawnActorClass != nullptr, TEXT("ProjectileActorClass is null on %s"), *GetName())) return;

    IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(GetAvatarActorFromActorInfo());
    if (CombatInterface)
    {
        const FVector SpawnLocation = CombatInterface->GetAbilitySpawnLocation();

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SpawnLocation);
        // TODO: Set the projectile rotation.

        AActor* OwnerActor = GetOwningActorFromActorInfo();
        APawn* InstigatorPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
        
        AGASCoreSpawnedActorByGameplayAbility* Projectile = GetWorld()->SpawnActorDeferred<AGASCoreSpawnedActorByGameplayAbility>(
            SpawnActorClass,
            SpawnTransform,
            OwnerActor,
            InstigatorPawn,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        // TODO: Give the projectile a gameplay effect spec for causing damage.
        
        Projectile->FinishSpawning(SpawnTransform);
    }
}
```

### Authority and Network Considerations

The spawning method includes several important network considerations:

1. **Server Authority**: Only spawns on server (`HasAuthority()` check)
2. **Deferred Spawning**: Uses `SpawnActorDeferred()` for initialization before placement
3. **Ownership**: Properly sets owner and instigator for network replication
4. **Collision Handling**: Uses `AlwaysSpawn` to ensure spawning succeeds

## Projectile Actor Implementation

### AGASCoreSpawnedActorByGameplayAbility Structure

```cpp
UCLASS()
class GASCORE_API AGASCoreSpawnedActorByGameplayAbility : public AActor
{
    GENERATED_BODY()
    
public:	
    AGASCoreSpawnedActorByGameplayAbility();

    UFUNCTION(BlueprintPure, Category = "GASCore|Projectile")
    UProjectileMovementComponent* GetProjectileMovementComponent() { return ProjectileMovementComponent; };

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult);

private:
    // Root scene component for designer attachment
    UPROPERTY(VisibleAnywhere, Category = "GASCore|Projectile|Components")
    TObjectPtr<USceneComponent> DefaultSceneRoot;

    // Collision detection
    UPROPERTY(VisibleAnywhere, Category = "GASCore|Projectile|Components")
    TObjectPtr<USphereComponent> SphereCollision;

    // Movement behavior
    UPROPERTY(VisibleAnywhere, Category = "GASCore|Projectile|Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
};
```

### Component Configuration

#### Scene Root Component
- Provides a root for attaching visual and collision components
- Allows designers to customize projectile appearance in Blueprint

#### Sphere Collision Component
- Handles collision detection with targets
- Configured for overlap events rather than blocking collision
- Triggers `OnSphereCollisionOverlap()` when hitting targets

#### Projectile Movement Component
- Provides physics-based projectile motion
- Handles gravity, bounce, and other movement properties
- Can be configured per projectile type

## Combat Interface Integration

### IGASCoreCombatInterface

```cpp
class GASCORE_API IGASCoreCombatInterface
{
public:
    // Get spawn location for projectiles/abilities
    virtual FVector GetAbilitySpawnLocation() const = 0;
    
    // Get actor level for scaling calculations
    virtual int32 GetActorLevel() const = 0;
    
    // Additional combat-related methods...
};
```

### Spawn Location Calculation

```cpp
// Example implementation in a character class
FVector AMyCharacter::GetAbilitySpawnLocation() const
{
    // Use weapon socket if available
    if (WeaponMeshComponent && WeaponMeshComponent->DoesSocketExist("Muzzle"))
    {
        return WeaponMeshComponent->GetSocketLocation("Muzzle");
    }
    
    // Fall back to hand socket
    if (GetMesh() && GetMesh()->DoesSocketExist("hand_r"))
    {
        return GetMesh()->GetSocketLocation("hand_r");
    }
    
    // Default to actor location with forward offset
    return GetActorLocation() + GetActorForwardVector() * 100.0f;
}
```

## Advanced Projectile Patterns

### Targeted Projectile Spawning

```cpp
void UGASCoreTargetedProjectileAbility::ActivateAbility(/*...*/)
{
    // Start target selection task
    UGASCoreTargetDataFromAimTrace* TargetTask = 
        UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(this);
    
    TargetTask->ValidHitResultData.AddDynamic(this, &UGASCoreTargetedProjectileAbility::OnTargetSelected);
    TargetTask->ReadyForActivation();
}

void UGASCoreTargetedProjectileAbility::OnTargetSelected(const FHitResult& HitResult)
{
    if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    // Store target location for projectile spawning
    TargetLocation = HitResult.Location;
    
    // Spawn projectile with targeting
    SpawnTargetedProjectile();
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGASCoreTargetedProjectileAbility::SpawnTargetedProjectile()
{
    // Get spawn location
    IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(GetAvatarActorFromActorInfo());
    if (!CombatInterface) return;
    
    const FVector SpawnLocation = CombatInterface->GetAbilitySpawnLocation();
    
    // Calculate direction to target
    FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
    FRotator SpawnRotation = Direction.Rotation();
    
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SpawnLocation);
    SpawnTransform.SetRotation(SpawnRotation.Quaternion());
    
    // Spawn projectile
    if (HasAuthority(&GetCurrentActivationInfo()))
    {
        AGASCoreSpawnedActorByGameplayAbility* Projectile = 
            GetWorld()->SpawnActorDeferred<AGASCoreSpawnedActorByGameplayAbility>(
                SpawnActorClass, SpawnTransform, 
                GetOwningActorFromActorInfo(), 
                Cast<APawn>(GetAvatarActorFromActorInfo()),
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        
        // Configure projectile movement toward target
        if (UProjectileMovementComponent* Movement = Projectile->GetProjectileMovementComponent())
        {
            Movement->Velocity = Direction * Movement->InitialSpeed;
        }
        
        Projectile->FinishSpawning(SpawnTransform);
    }
}
```

### Multi-Projectile Spawning

```cpp
void UGASCoreMultiProjectileAbility::SpawnActorFromGameplayAbility()
{
    const FGameplayAbilityActivationInfo GameplayAbilityActivationInfo = GetCurrentActivationInfo();
    const bool bIsServer = HasAuthority(&GameplayAbilityActivationInfo);
    if (!bIsServer) return;

    IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(GetAvatarActorFromActorInfo());
    if (!CombatInterface) return;

    const FVector SpawnLocation = CombatInterface->GetAbilitySpawnLocation();
    const FVector ForwardVector = GetAvatarActorFromActorInfo()->GetActorForwardVector();
    
    // Spawn multiple projectiles in a spread pattern
    for (int32 i = 0; i < ProjectileCount; ++i)
    {
        // Calculate spread angle
        float AngleOffset = 0.0f;
        if (ProjectileCount > 1)
        {
            float SpreadRange = FMath::DegreesToRadians(SpreadAngle);
            AngleOffset = FMath::Lerp(-SpreadRange / 2.0f, SpreadRange / 2.0f, 
                float(i) / float(ProjectileCount - 1));
        }
        
        // Rotate direction by spread angle
        FVector ProjectileDirection = ForwardVector.RotateAngleAxis(
            FMath::RadiansToDegrees(AngleOffset), FVector::UpVector);
        
        FRotator SpawnRotation = ProjectileDirection.Rotation();
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SpawnLocation);
        SpawnTransform.SetRotation(SpawnRotation.Quaternion());
        
        // Spawn individual projectile
        AGASCoreSpawnedActorByGameplayAbility* Projectile = 
            GetWorld()->SpawnActorDeferred<AGASCoreSpawnedActorByGameplayAbility>(
                SpawnActorClass, SpawnTransform,
                GetOwningActorFromActorInfo(),
                Cast<APawn>(GetAvatarActorFromActorInfo()),
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        
        Projectile->FinishSpawning(SpawnTransform);
    }
}
```

### Homing Projectiles

```cpp
// Enhanced projectile class with homing capability
UCLASS()
class GASCORE_API AGASCoreHomingProjectile : public AGASCoreSpawnedActorByGameplayAbility
{
    GENERATED_BODY()

public:
    AGASCoreHomingProjectile();

    // Set target for homing behavior
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SetHomingTarget(AActor* Target);

protected:
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    TWeakObjectPtr<AActor> HomingTarget;

    UPROPERTY(EditDefaultsOnly, Category = "Homing")
    float HomingAcceleration = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Homing")
    float MaxHomingDistance = 2000.0f;
};

void AGASCoreHomingProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (HomingTarget.IsValid() && ProjectileMovementComponent)
    {
        FVector TargetLocation = HomingTarget->GetActorLocation();
        FVector CurrentLocation = GetActorLocation();
        float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
        
        // Only home if within max distance
        if (DistanceToTarget <= MaxHomingDistance)
        {
            FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
            FVector CurrentVelocity = ProjectileMovementComponent->Velocity;
            
            // Apply homing acceleration
            FVector HomingForce = Direction * HomingAcceleration * DeltaTime;
            ProjectileMovementComponent->Velocity = (CurrentVelocity + HomingForce).GetClampedToMaxSize(
                ProjectileMovementComponent->MaxSpeed);
        }
    }
}
```

## Collision and Damage Integration

### Collision Handling

```cpp
void AGASCoreSpawnedActorByGameplayAbility::OnSphereCollisionOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Ignore self and owner
    if (OtherActor == this || OtherActor == GetOwner())
        return;
    
    // Check if target is valid
    if (!IsValidTarget(OtherActor))
        return;
    
    // Apply damage effect
    ApplyDamageEffect(OtherActor);
    
    // Trigger impact effects
    OnProjectileImpact(SweepResult);
    
    // Destroy projectile
    Destroy();
}

bool AGASCoreSpawnedActorByGameplayAbility::IsValidTarget(AActor* Target) const
{
    // Implement target validation logic
    if (!Target)
        return false;
    
    // Check team affiliation
    if (IGASCoreCombatInterface* TargetCombat = Cast<IGASCoreCombatInterface>(Target))
    {
        if (IGASCoreCombatInterface* OwnerCombat = Cast<IGASCoreCombatInterface>(GetOwner()))
        {
            // Don't damage allies (simplified check)
            return TargetCombat->GetActorLevel() != OwnerCombat->GetActorLevel();
        }
    }
    
    return true;
}
```

### Damage Effect Application

```cpp
void AGASCoreSpawnedActorByGameplayAbility::ApplyDamageEffect(AActor* Target)
{
    if (!DamageEffectSpecHandle.IsValid())
        return;
    
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
    {
        if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
        {
            // Apply the damage effect to target
            FActiveGameplayEffectHandle ActiveHandle = TargetASC->ApplyGameplayEffectSpecToSelf(
                *DamageEffectSpecHandle.Data.Get());
            
            UE_LOG(LogAbilitySystem, Log, TEXT("Projectile applied damage effect to %s"), 
                *GetNameSafe(Target));
        }
    }
}
```

## Configuration and Customization

### Blueprint Configuration

```cpp
// Expose configuration properties for Blueprint setup
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API UGASCoreProjectileAbility : public UGASCoreGameplayAbility
{
    GENERATED_BODY()

protected:
    // Projectile configuration
    UPROPERTY(EditDefaultsOnly, Category = "Projectile", 
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ProjectileSpeed = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ProjectileGravity = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    bool bShouldBounce = false;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float BounceFriction = 0.3f;

    // Damage configuration
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float BaseDamage = 50.0f;

    // Spawning configuration
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    FVector SpawnOffset = FVector(100.0f, 0.0f, 0.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    FName SpawnSocketName = "hand_r";
};
```

### Runtime Configuration

```cpp
void UGASCoreProjectileAbility::ConfigureProjectile(AGASCoreSpawnedActorByGameplayAbility* Projectile)
{
    if (!Projectile) return;

    // Configure movement component
    if (UProjectileMovementComponent* Movement = Projectile->GetProjectileMovementComponent())
    {
        Movement->InitialSpeed = ProjectileSpeed;
        Movement->MaxSpeed = ProjectileSpeed;
        Movement->ProjectileGravityScale = ProjectileGravity;
        Movement->bShouldBounce = bShouldBounce;
        Movement->Friction = BounceFriction;
    }

    // Set damage effect spec
    if (DamageEffectClass)
    {
        FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
        EffectContext.SetAbility(this);
        EffectContext.AddSourceObject(this);
        EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

        FGameplayEffectSpecHandle EffectSpec = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
            DamageEffectClass, GetAbilityLevel(), EffectContext);

        if (EffectSpec.IsValid())
        {
            // Set damage magnitude
            EffectSpec.Data->SetSetByCallerMagnitude(
                FGameplayTag::RequestGameplayTag("Data.Damage"), BaseDamage);
            
            Projectile->SetDamageEffectSpec(EffectSpec);
        }
    }
}
```

## Performance Optimization

### Object Pooling

```cpp
class GASCORE_API UProjectilePool : public UGameObject
{
public:
    static UProjectilePool* Get(UWorld* World);
    
    AGASCoreSpawnedActorByGameplayAbility* GetPooledProjectile(
        TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> ProjectileClass);
    
    void ReturnProjectile(AGASCoreSpawnedActorByGameplayAbility* Projectile);

private:
    TMap<TSubclassOf<AGASCoreSpawnedActorByGameplayAbility>, 
         TArray<AGASCoreSpawnedActorByGameplayAbility*>> ProjectilePools;
};
```

### Lifetime Management

```cpp
void AGASCoreSpawnedActorByGameplayAbility::BeginPlay()
{
    Super::BeginPlay();
    
    // Set automatic cleanup timer
    FTimerHandle LifetimeTimer;
    GetWorldTimerManager().SetTimer(LifetimeTimer, this, 
        &AGASCoreSpawnedActorByGameplayAbility::OnLifetimeExpired, 
        ProjectileLifetime, false);
}

void AGASCoreSpawnedActorByGameplayAbility::OnLifetimeExpired()
{
    // Return to pool or destroy
    if (UProjectilePool* Pool = UProjectilePool::Get(GetWorld()))
    {
        Pool->ReturnProjectile(this);
    }
    else
    {
        Destroy();
    }
}
```

## Testing and Debugging

### Debug Visualization

```cpp
void AGASCoreSpawnedActorByGameplayAbility::DebugDraw()
{
#if !UE_BUILD_SHIPPING
    if (CVarDebugProjectiles.GetValueOnGameThread())
    {
        // Draw projectile path
        if (ProjectileMovementComponent)
        {
            FVector Start = GetActorLocation();
            FVector End = Start + ProjectileMovementComponent->Velocity.GetSafeNormal() * 1000.0f;
            
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);
        }
        
        // Draw collision sphere
        if (SphereCollision)
        {
            DrawDebugSphere(GetWorld(), GetActorLocation(), 
                SphereCollision->GetScaledSphereRadius(), 12, FColor::Green, false, 0.1f);
        }
    }
#endif
}
```

### Console Commands

```cpp
// In your game module
static TAutoConsoleVariable<bool> CVarDebugProjectiles(
    TEXT("gas.debug.projectiles"),
    false,
    TEXT("Enable projectile debug visualization"));

static TAutoConsoleVariable<bool> CVarLogProjectileSpawning(
    TEXT("gas.log.projectile.spawning"),
    false,
    TEXT("Log projectile spawning events"));
```

## Related Documentation

- [Gameplay Ability Tasks](gameplay-ability-tasks.md)
- [Ability Task Montage Integration](ability-task-montage.md)
- [Projectile Abilities Pattern](projectile-abilities.md)
- [Combat Interface](combat-interface.md)
- [Gameplay Abilities Concepts and Practice](../guides/GameplayAbilities_Concepts_And_Practice.md)

## See Also

- [Unreal Engine Projectile Movement Component](https://docs.unrealengine.com/5.3/en-US/projectile-movement-component-in-unreal-engine/)
- [Unreal Engine Actor Spawning](https://docs.unrealengine.com/5.3/en-US/spawning-actors-in-unreal-engine/)
- [Gameplay Ability System Documentation](https://docs.unrealengine.com/5.3/en-US/gameplay-ability-system-for-unreal-engine/)