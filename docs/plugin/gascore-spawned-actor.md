# GASCore Spawned Actor System

Last updated: 2025-01-21

## Overview

The `AGASCoreSpawnedActorByGameplayAbility` class provides the foundation for actors spawned by gameplay abilities in the GASCore plugin. This system is designed specifically for projectiles, temporary objects, and other dynamic actors that need tight integration with the Gameplay Ability System.

The spawned actor system handles collision detection, movement, lifetime management, and effect application in a network-replicated environment.

## Core Architecture

### Class Hierarchy

```
AActor
└─ AGASCoreSpawnedActorByGameplayAbility
   ├─ AGASCoreProjectileActor (derived projectile types)
   ├─ AGASCoreSummonedActor (summoned creatures/objects)
   └─ AGASCoreTemporaryActor (temporary effects/objects)
```

### Component Structure

```cpp
AGASCoreSpawnedActorByGameplayAbility
├─ USceneComponent* DefaultSceneRoot           // Root component for attachments
├─ USphereComponent* SphereCollision           // Collision detection
└─ UProjectileMovementComponent* ProjectileMovementComponent  // Movement behavior
```

## Class Implementation

### Header Definition

```cpp
// GASCoreSpawnedActorByGameplayAbility.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GASCoreSpawnedActorByGameplayAbility.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UGameplayEffect;

/**
 * Base class for actors spawned by gameplay abilities.
 * Provides collision detection, movement, and effect application.
 */
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API AGASCoreSpawnedActorByGameplayAbility : public AActor
{
    GENERATED_BODY()
    
public:	
    AGASCoreSpawnedActorByGameplayAbility();

    // Accessors
    UFUNCTION(BlueprintPure, Category = "GASCore|Projectile")
    UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }

    UFUNCTION(BlueprintPure, Category = "GASCore|Projectile")
    USphereComponent* GetSphereCollision() const { return SphereCollision; }

    // Effect application
    UFUNCTION(BlueprintCallable, Category = "GASCore|Effects")
    void SetDamageEffectSpec(const FGameplayEffectSpecHandle& InDamageEffect);

    UFUNCTION(BlueprintCallable, Category = "GASCore|Effects")
    void ApplyEffectToTarget(AActor* Target);

    // Lifetime management
    UFUNCTION(BlueprintCallable, Category = "GASCore|Lifetime")
    void SetLifetime(float InLifetime);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Collision handling
    UFUNCTION()
    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult);

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "GASCore|Events")
    void OnActorHit(AActor* HitActor, const FHitResult& HitResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "GASCore|Events")
    void OnLifetimeExpired();

    // Target validation
    UFUNCTION(BlueprintNativeEvent, Category = "GASCore|Targeting")
    bool IsValidTarget(AActor* PotentialTarget) const;
    virtual bool IsValidTarget_Implementation(AActor* PotentialTarget) const;

    // Effect application
    virtual void ApplyDamageEffect(AActor* Target);
    virtual void ApplyAreaEffect(const FVector& Location, float Radius);

    // Lifetime management
    UFUNCTION()
    virtual void HandleLifetimeExpired();

private:
    // -----------------------------------------------------------------------
    // COMPONENTS
    // -----------------------------------------------------------------------

    /** Root scene component for designer attachments */
    UPROPERTY(VisibleAnywhere, Category = "GASCore|Projectile|Components")
    TObjectPtr<USceneComponent> DefaultSceneRoot;

    /** Collision component for overlap detection */
    UPROPERTY(VisibleAnywhere, Category = "GASCore|Projectile|Components")
    TObjectPtr<USphereComponent> SphereCollision;

    /** Movement component for projectile motion */
    UPROPERTY(VisibleAnywhere, Category = "GASCore|Projectile|Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

    // -----------------------------------------------------------------------
    // CONFIGURATION
    // -----------------------------------------------------------------------

    /** Maximum lifetime before auto-destruction */
    UPROPERTY(EditDefaultsOnly, Category = "GASCore|Lifetime")
    float MaxLifetime = 10.0f;

    /** Whether to destroy on any collision */
    UPROPERTY(EditDefaultsOnly, Category = "GASCore|Collision")
    bool bDestroyOnHit = true;

    /** Whether to apply effects to multiple targets */
    UPROPERTY(EditDefaultsOnly, Category = "GASCore|Effects")
    bool bCanHitMultipleTargets = false;

    /** Collision sphere radius */
    UPROPERTY(EditDefaultsOnly, Category = "GASCore|Collision")
    float CollisionRadius = 5.0f;

    // -----------------------------------------------------------------------
    // RUNTIME DATA
    // -----------------------------------------------------------------------

    /** Damage effect to apply on hit */
    UPROPERTY()
    FGameplayEffectSpecHandle DamageEffectSpecHandle;

    /** Actors already hit (for multi-target prevention) */
    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> HitActors;

    /** Lifetime timer handle */
    FTimerHandle LifetimeTimerHandle;
};
```

### Implementation

```cpp
// GASCoreSpawnedActorByGameplayAbility.cpp
#include "Actors/GASCoreSpawnedActorByGameplayAbility.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AGASCoreSpawnedActorByGameplayAbility::AGASCoreSpawnedActorByGameplayAbility()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // Create root component
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Create collision component
    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    SphereCollision->SetupAttachment(RootComponent);
    SphereCollision->SetSphereRadius(CollisionRadius);
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SphereCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    SphereCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

    // Bind collision event
    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, 
        &AGASCoreSpawnedActorByGameplayAbility::OnSphereCollisionOverlap);

    // Create projectile movement component
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 1000.0f;
    ProjectileMovementComponent->MaxSpeed = 1000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bShouldBounce = false;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AGASCoreSpawnedActorByGameplayAbility::BeginPlay()
{
    Super::BeginPlay();

    // Set up lifetime timer
    if (MaxLifetime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, 
            &AGASCoreSpawnedActorByGameplayAbility::HandleLifetimeExpired, 
            MaxLifetime, false);
    }

    // Update collision radius if changed
    if (SphereCollision && CollisionRadius > 0.0f)
    {
        SphereCollision->SetSphereRadius(CollisionRadius);
    }
}

void AGASCoreSpawnedActorByGameplayAbility::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear lifetime timer
    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}

void AGASCoreSpawnedActorByGameplayAbility::OnSphereCollisionOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Ignore invalid actors
    if (!OtherActor || OtherActor == this)
        return;

    // Ignore owner
    if (OtherActor == GetOwner())
        return;

    // Check if we can hit this target
    if (!IsValidTarget(OtherActor))
        return;

    // Check for duplicate hits
    if (!bCanHitMultipleTargets && HitActors.Contains(OtherActor))
        return;

    // Record hit
    HitActors.Add(OtherActor);

    UE_LOG(LogTemp, Log, TEXT("Spawned actor hit: %s"), *GetNameSafe(OtherActor));

    // Apply effects
    ApplyEffectToTarget(OtherActor);

    // Fire Blueprint event
    OnActorHit(OtherActor, SweepResult);

    // Destroy if configured to do so
    if (bDestroyOnHit)
    {
        Destroy();
    }
}

bool AGASCoreSpawnedActorByGameplayAbility::IsValidTarget_Implementation(AActor* PotentialTarget) const
{
    if (!PotentialTarget)
        return false;

    // Must have ability system component to be targetable
    if (!Cast<IAbilitySystemInterface>(PotentialTarget))
        return false;

    // Don't target self or owner
    if (PotentialTarget == this || PotentialTarget == GetOwner())
        return false;

    // Add additional validation logic here
    return true;
}

void AGASCoreSpawnedActorByGameplayAbility::SetDamageEffectSpec(const FGameplayEffectSpecHandle& InDamageEffect)
{
    DamageEffectSpecHandle = InDamageEffect;
}

void AGASCoreSpawnedActorByGameplayAbility::ApplyEffectToTarget(AActor* Target)
{
    if (!Target)
        return;

    ApplyDamageEffect(Target);
}

void AGASCoreSpawnedActorByGameplayAbility::ApplyDamageEffect(AActor* Target)
{
    if (!DamageEffectSpecHandle.IsValid())
        return;

    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
    {
        if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
        {
            FActiveGameplayEffectHandle ActiveHandle = TargetASC->ApplyGameplayEffectSpecToSelf(
                *DamageEffectSpecHandle.Data.Get());

            UE_LOG(LogTemp, Log, TEXT("Applied damage effect to %s"), *GetNameSafe(Target));
        }
    }
}

void AGASCoreSpawnedActorByGameplayAbility::SetLifetime(float InLifetime)
{
    MaxLifetime = InLifetime;

    // Update timer if already started
    if (GetWorld() && MaxLifetime > 0.0f)
    {
        GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
        GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this,
            &AGASCoreSpawnedActorByGameplayAbility::HandleLifetimeExpired,
            MaxLifetime, false);
    }
}

void AGASCoreSpawnedActorByGameplayAbility::HandleLifetimeExpired()
{
    UE_LOG(LogTemp, Log, TEXT("Spawned actor lifetime expired: %s"), *GetName());

    // Fire Blueprint event
    OnLifetimeExpired();

    // Destroy actor
    Destroy();
}
```

## Usage Patterns

### Basic Projectile Creation

```cpp
// In a gameplay ability
void UMyProjectileAbility::SpawnProjectile()
{
    if (!HasAuthority(&GetCurrentActivationInfo()))
        return;

    // Get spawn location
    FVector SpawnLocation = GetSpawnLocation();
    FVector TargetLocation = GetTargetLocation();
    FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();

    // Set up spawn transform
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SpawnLocation);
    SpawnTransform.SetRotation(Direction.Rotation().Quaternion());

    // Spawn projectile
    AGASCoreSpawnedActorByGameplayAbility* Projectile = GetWorld()->SpawnActorDeferred<AGASCoreSpawnedActorByGameplayAbility>(
        ProjectileClass, SpawnTransform,
        GetOwningActorFromActorInfo(),
        Cast<APawn>(GetAvatarActorFromActorInfo()),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    // Configure projectile
    if (Projectile)
    {
        // Set damage effect
        FGameplayEffectSpecHandle DamageSpec = CreateDamageEffectSpec();
        Projectile->SetDamageEffectSpec(DamageSpec);

        // Configure movement
        if (UProjectileMovementComponent* Movement = Projectile->GetProjectileMovementComponent())
        {
            Movement->Velocity = Direction * ProjectileSpeed;
        }

        // Finish spawning
        Projectile->FinishSpawning(SpawnTransform);
    }
}
```

### Blueprint Spawning

```cpp
// Blueprint-callable spawning function
UFUNCTION(BlueprintCallable, Category = "GASCore|Spawning", CallInEditor = true)
static AGASCoreSpawnedActorByGameplayAbility* SpawnActorFromBlueprint(
    UObject* WorldContextObject,
    TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> ActorClass,
    const FTransform& SpawnTransform,
    AActor* Owner = nullptr,
    APawn* Instigator = nullptr)
{
    if (!WorldContextObject || !ActorClass)
        return nullptr;

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
        return nullptr;

    AGASCoreSpawnedActorByGameplayAbility* SpawnedActor = World->SpawnActorDeferred<AGASCoreSpawnedActorByGameplayAbility>(
        ActorClass, SpawnTransform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (SpawnedActor)
    {
        SpawnedActor->FinishSpawning(SpawnTransform);
    }

    return SpawnedActor;
}
```

## Advanced Features

### Area of Effect (AOE) Support

```cpp
void AGASCoreSpawnedActorByGameplayAbility::ApplyAreaEffect(const FVector& Location, float Radius)
{
    if (!GetWorld())
        return;

    // Find all overlapping actors
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetOwner());

    bool bFoundOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Location,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bFoundOverlaps)
    {
        for (const FOverlapResult& Overlap : OverlapResults)
        {
            if (AActor* OverlappedActor = Overlap.GetActor())
            {
                if (IsValidTarget(OverlappedActor))
                {
                    ApplyEffectToTarget(OverlappedActor);
                }
            }
        }
    }

    // Visual effect for AOE
    OnAreaEffectApplied(Location, Radius);
}

// Blueprint implementable event for visual effects
UFUNCTION(BlueprintImplementableEvent, Category = "GASCore|Effects")
void OnAreaEffectApplied(const FVector& Location, float Radius);
```

### Bouncing Projectiles

```cpp
UCLASS()
class GASCORE_API AGASCoreBouncingProjectile : public AGASCoreSpawnedActorByGameplayAbility
{
    GENERATED_BODY()

public:
    AGASCoreBouncingProjectile();

protected:
    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult) override;

    UFUNCTION()
    void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Bouncing")
    int32 MaxBounces = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Bouncing")
    float BounceDecayFactor = 0.8f;

    int32 CurrentBounces = 0;
};

AGASCoreBouncingProjectile::AGASCoreBouncingProjectile()
{
    // Configure for bouncing
    if (ProjectileMovementComponent)
    {
        ProjectileMovementComponent->bShouldBounce = true;
        ProjectileMovementComponent->Bounciness = 0.6f;
        ProjectileMovementComponent->Friction = 0.2f;
        
        // Bind to hit event for bounce counting
        ProjectileMovementComponent->OnProjectileHit.AddDynamic(this, 
            &AGASCoreBouncingProjectile::OnProjectileHit);
    }

    // Don't destroy on hit for bouncing projectiles
    bDestroyOnHit = false;
}

void AGASCoreBouncingProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, 
    FVector NormalImpulse, const FHitResult& Hit)
{
    CurrentBounces++;

    // Reduce speed on bounce
    if (ProjectileMovementComponent)
    {
        FVector NewVelocity = ProjectileMovementComponent->Velocity * BounceDecayFactor;
        ProjectileMovementComponent->Velocity = NewVelocity;
    }

    // Check if max bounces reached
    if (CurrentBounces >= MaxBounces)
    {
        Destroy();
    }
}
```

### Guided Projectiles

```cpp
UCLASS()
class GASCORE_API AGASCoreGuidedProjectile : public AGASCoreSpawnedActorByGameplayAbility
{
    GENERATED_BODY()

public:
    AGASCoreGuidedProjectile();

    UFUNCTION(BlueprintCallable, Category = "Guided")
    void SetHomingTarget(AActor* Target);

protected:
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    TWeakObjectPtr<AActor> HomingTarget;

    UPROPERTY(EditDefaultsOnly, Category = "Homing")
    float HomingStrength = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Homing")
    float MaxHomingDistance = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Homing")
    float HomingDelay = 0.5f;

    float HomingTimer = 0.0f;
};

AGASCoreGuidedProjectile::AGASCoreGuidedProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGASCoreGuidedProjectile::SetHomingTarget(AActor* Target)
{
    HomingTarget = Target;
}

void AGASCoreGuidedProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    HomingTimer += DeltaTime;

    // Only start homing after delay
    if (HomingTimer < HomingDelay)
        return;

    if (!HomingTarget.IsValid() || !ProjectileMovementComponent)
        return;

    FVector TargetLocation = HomingTarget->GetActorLocation();
    FVector CurrentLocation = GetActorLocation();
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    // Only home if within max distance
    if (DistanceToTarget > MaxHomingDistance)
        return;

    // Calculate homing direction
    FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector CurrentVelocity = ProjectileMovementComponent->Velocity;
    FVector CurrentDirection = CurrentVelocity.GetSafeNormal();

    // Apply homing force
    FVector HomingForce = ToTarget * HomingStrength * DeltaTime;
    FVector NewVelocity = CurrentVelocity + HomingForce;

    // Maintain original speed
    NewVelocity = NewVelocity.GetSafeNormal() * CurrentVelocity.Size();

    ProjectileMovementComponent->Velocity = NewVelocity;
}
```

## Network Replication

### Replication Setup

```cpp
void AGASCoreSpawnedActorByGameplayAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate damage effect spec handle
    DOREPLIFETIME_CONDITION(AGASCoreSpawnedActorByGameplayAbility, DamageEffectSpecHandle, 
        COND_InitialOnly);

    // Replicate configuration
    DOREPLIFETIME_CONDITION(AGASCoreSpawnedActorByGameplayAbility, MaxLifetime, 
        COND_InitialOnly);
    DOREPLIFETIME_CONDITION(AGASCoreSpawnedActorByGameplayAbility, bDestroyOnHit, 
        COND_InitialOnly);
}
```

### Client Prediction

```cpp
void AGASCoreSpawnedActorByGameplayAbility::OnSphereCollisionOverlap(/*...*/)
{
    // Only apply effects on server
    if (HasAuthority())
    {
        ApplyEffectToTarget(OtherActor);
    }

    // Visual effects can run on all clients
    OnActorHit(OtherActor, SweepResult);

    // Only server decides destruction
    if (HasAuthority() && bDestroyOnHit)
    {
        Destroy();
    }
}
```

## Performance Optimization

### Object Pooling System

```cpp
UCLASS()
class GASCORE_API UGASCoreSpawnedActorPool : public UObject
{
    GENERATED_BODY()

public:
    static UGASCoreSpawnedActorPool* Get(UWorld* World);

    AGASCoreSpawnedActorByGameplayAbility* GetPooledActor(
        TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> ActorClass);

    void ReturnActor(AGASCoreSpawnedActorByGameplayAbility* Actor);

private:
    TMap<TSubclassOf<AGASCoreSpawnedActorByGameplayAbility>, 
         TArray<AGASCoreSpawnedActorByGameplayAbility*>> ActorPools;

    void InitializePool(TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> ActorClass, 
        int32 PoolSize = 50);
};
```

### Optimized Collision Detection

```cpp
void AGASCoreSpawnedActorByGameplayAbility::OptimizeForPerformance()
{
    // Reduce tick frequency for less critical projectiles
    PrimaryActorTick.TickInterval = 0.1f;

    // Use simple collision only
    if (SphereCollision)
    {
        SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
        SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }

    // Disable unnecessary components for performance
    if (ProjectileMovementComponent)
    {
        ProjectileMovementComponent->bRotationFollowsVelocity = false;
        ProjectileMovementComponent->bSimulationEnabled = true;
    }
}
```

## Testing and Debugging

### Debug Visualization

```cpp
void AGASCoreSpawnedActorByGameplayAbility::DrawDebugInfo()
{
#if !UE_BUILD_SHIPPING
    if (CVarDebugSpawnedActors.GetValueOnGameThread())
    {
        // Draw collision sphere
        if (SphereCollision)
        {
            DrawDebugSphere(GetWorld(), GetActorLocation(), 
                SphereCollision->GetScaledSphereRadius(), 12, FColor::Green, false, 0.1f);
        }

        // Draw velocity vector
        if (ProjectileMovementComponent)
        {
            FVector Velocity = ProjectileMovementComponent->Velocity;
            DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(),
                GetActorLocation() + Velocity.GetSafeNormal() * 100.0f,
                50.0f, FColor::Red, false, 0.1f);
        }

        // Draw lifetime remaining
        float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LifetimeTimerHandle);
        if (RemainingTime > 0.0f)
        {
            FString LifetimeText = FString::Printf(TEXT("%.1fs"), RemainingTime);
            DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 50),
                LifetimeText, nullptr, FColor::Yellow, 0.1f);
        }
    }
#endif
}
```

### Console Commands

```cpp
// Add to your game module
static TAutoConsoleVariable<bool> CVarDebugSpawnedActors(
    TEXT("gas.debug.spawnedactors"),
    false,
    TEXT("Enable spawned actor debug visualization"));

static TAutoConsoleVariable<bool> CVarLogSpawnedActors(
    TEXT("gas.log.spawnedactors"),
    false,
    TEXT("Log spawned actor events"));
```

## Related Documentation

- [Spawn Projectile System](spawn-projectile.md)
- [Gameplay Ability Tasks](gameplay-ability-tasks.md)
- [Projectile Abilities Pattern](projectile-abilities.md)
- [Combat Interface](combat-interface.md)
- [Gameplay Abilities Concepts and Practice](../guides/GameplayAbilities_Concepts_And_Practice.md)

## See Also

- [Unreal Engine Actor Lifecycle](https://docs.unrealengine.com/5.3/en-US/actor-lifecycle-in-unreal-engine/)
- [Unreal Engine Projectile Movement](https://docs.unrealengine.com/5.3/en-US/projectile-movement-component-in-unreal-engine/)
- [Unreal Engine Collision Detection](https://docs.unrealengine.com/5.3/en-US/collision-in-unreal-engine/)