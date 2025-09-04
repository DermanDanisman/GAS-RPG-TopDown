# Projectile Abilities Pattern

Last updated: 2024-12-19

## Overview

Projectile abilities represent a common gameplay pattern where abilities spawn moving actors (projectiles) that travel to targets and apply effects on impact. This pattern integrates the Gameplay Ability System with the projectile actor framework for consistent, replicable spell/attack mechanics.

## Core Pattern Architecture

### Base Projectile Ability

While `GASCoreProjectileAbility` doesn't exist as a specific class, the pattern follows the established `UGASCoreGameplayAbility` base with projectile-specific functionality:

```cpp
UCLASS(Abstract, BlueprintType, Blueprintable)
class GASCORE_API UProjectileAbilityBase : public UGASCoreGameplayAbility
{
    GENERATED_BODY()

public:
    UProjectileAbilityBase();

protected:
    // ================================================================================================================
    // PROJECTILE CONFIGURATION
    // ================================================================================================================
    
    /** Projectile actor class to spawn */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TSubclassOf<class AGASCoreProjectileActor> ProjectileClass;
    
    /** Speed override for projectile (if different from actor default) */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ProjectileSpeed = 0.f;
    
    /** Maximum range before projectile auto-destructs */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float MaxRange = 1000.f;
    
    /** Spawn offset from character (relative to forward direction) */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    FVector SpawnOffset = FVector(100.f, 0.f, 0.f);

    // ================================================================================================================
    // TARGETING
    // ================================================================================================================
    
    /** How to determine projectile target/direction */
    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    EProjectileTargetingMode TargetingMode = EProjectileTargetingMode::CursorLocation;
    
    /** Range for target acquisition (used with targeting modes) */
    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    float TargetingRange = 1500.f;

    // ================================================================================================================
    // ABILITY IMPLEMENTATION
    // ================================================================================================================
    
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
    
    /** Calculate spawn location based on character position and spawn offset */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual FVector CalculateSpawnLocation() const;
    
    /** Determine target location based on targeting mode */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual FVector CalculateTargetLocation() const;
    
    /** Spawn the projectile actor with proper parameters */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual AActor* SpawnProjectile(const FVector& StartLocation, const FVector& TargetLocation);
    
    /** Validate that projectile can be spawned (has valid class, location, etc.) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Projectile")
    virtual bool CanSpawnProjectile() const;
};
```

### Targeting Modes

```cpp
UENUM(BlueprintType)
enum class EProjectileTargetingMode : uint8
{
    /** Target current cursor/crosshair location */
    CursorLocation,
    
    /** Target closest enemy within range */
    ClosestEnemy,
    
    /** Target in forward direction from character */
    ForwardDirection,
    
    /** Target specific actor (set via targeting task) */
    SpecificActor,
    
    /** Fire in last movement direction */
    MovementDirection
};
```

## Implementation Examples

### Basic Firebolt Ability

```cpp
void UFireboltAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Calculate spawn and target locations
    FVector SpawnLocation = CalculateSpawnLocation();
    FVector TargetLocation = CalculateTargetLocation();
    
    // Validate trajectory
    if (!IsValidTrajectory(SpawnLocation, TargetLocation))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
    // Spawn projectile on server
    if (HasAuthority(&ActivationInfo))
    {
        AActor* Projectile = SpawnProjectile(SpawnLocation, TargetLocation);
        if (Projectile)
        {
            // Store projectile reference for potential cancellation
            CurrentProjectile = Projectile;
        }
    }
    
    // Play casting animation/effects locally
    PlayCastingEffects();
    
    // End ability (projectile handles impact independently)
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
```

### Multi-Projectile Ability (Shotgun Pattern)

```cpp
void UMultiProjectileAbility::SpawnProjectileSpread(const FVector& BaseDirection, int32 ProjectileCount, float SpreadAngle)
{
    if (!HasAuthority()) return;
    
    FVector SpawnLocation = CalculateSpawnLocation();
    
    for (int32 i = 0; i < ProjectileCount; i++)
    {
        // Calculate spread offset
        float AngleOffset = FMath::Lerp(-SpreadAngle/2.f, SpreadAngle/2.f, 
            ProjectileCount > 1 ? (float)i / (ProjectileCount - 1) : 0.f);
        
        // Rotate base direction by offset
        FRotator SpreadRotation = BaseDirection.Rotation();
        SpreadRotation.Yaw += AngleOffset;
        FVector ProjectileDirection = SpreadRotation.Vector();
        
        // Calculate target location
        FVector TargetLocation = SpawnLocation + (ProjectileDirection * MaxRange);
        
        // Spawn individual projectile
        SpawnProjectile(SpawnLocation, TargetLocation);
        
        // Optional: Add slight delay between projectiles
        if (ProjectileSpawnDelay > 0.f && i < ProjectileCount - 1)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SpawnLocation, TargetLocation]()
            {
                SpawnProjectile(SpawnLocation, TargetLocation);
            }, ProjectileSpawnDelay, false);
        }
    }
}
```

### Guided Projectile with Targeting Task

```cpp
void UGuidedProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // Start target selection task
    UAbilityTask_WaitTargetData* TargetTask = UAbilityTask_WaitTargetData::WaitTargetData(
        this, TEXT("SelectTarget"), EGameplayTargetingConfirmation::UserConfirmed, 
        CreateTargetingClass());
    
    TargetTask->ValidData.AddDynamic(this, &UGuidedProjectileAbility::OnTargetSelected);
    TargetTask->Cancelled.AddDynamic(this, &UGuidedProjectileAbility::OnTargetingCancelled);
    TargetTask->ReadyForActivation();
}

void UGuidedProjectileAbility::OnTargetSelected(const FGameplayAbilityTargetDataHandle& TargetData)
{
    if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    // Extract target location from targeting data
    FVector TargetLocation = TargetData.Get(0)->GetHitResult()->Location;
    FVector SpawnLocation = CalculateSpawnLocation();
    
    // Spawn guided projectile with target reference
    if (HasAuthority(&CurrentActivationInfo))
    {
        if (AGuidedProjectile* GuidedProjectile = Cast<AGuidedProjectile>(
            SpawnProjectile(SpawnLocation, TargetLocation)))
        {
            // Set target for homing behavior
            if (AActor* TargetActor = TargetData.Get(0)->GetHitResult()->GetActor())
            {
                GuidedProjectile->SetHomingTarget(TargetActor);
            }
        }
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

## Advanced Patterns

### Projectile with Payload Effects

```cpp
class GASCORE_API APayloadProjectile : public AGASCoreProjectileActor
{
    GENERATED_BODY()

protected:
    /** Gameplay effect to apply on impact */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> ImpactEffect;
    
    /** Additional effects for area of effect */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> AOEEffect;
    
    /** Radius for area effect application */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    float AOERadius = 200.f;
    
    /** Context information from spawning ability */
    UPROPERTY(BlueprintReadOnly, Category = "Context")
    FGameplayEffectContextHandle AbilityContext;

    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult) override;
    
    /** Apply primary impact effect to hit target */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ApplyImpactEffect(AActor* Target);
    
    /** Apply area effect to nearby targets */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ApplyAreaEffect(const FVector& ImpactLocation);
    
public:
    /** Set the ability context for effect application */
    UFUNCTION(BlueprintCallable, Category = "Context")
    void SetAbilityContext(const FGameplayEffectContextHandle& Context) { AbilityContext = Context; }
};
```

### Bouncing Projectile Pattern

```cpp
class GASCORE_API ABouncingProjectile : public AGASCoreProjectileActor
{
    GENERATED_BODY()

protected:
    /** Maximum number of bounces */
    UPROPERTY(EditDefaultsOnly, Category = "Bouncing")
    int32 MaxBounces = 3;
    
    /** Current bounce count */
    UPROPERTY(BlueprintReadOnly, Category = "Bouncing")
    int32 CurrentBounces = 0;
    
    /** Speed reduction per bounce */
    UPROPERTY(EditDefaultsOnly, Category = "Bouncing")
    float BounceSpeedModifier = 0.8f;
    
    /** Range for finding bounce targets */
    UPROPERTY(EditDefaultsOnly, Category = "Bouncing")
    float BounceRange = 300.f;

    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult) override;
    
    /** Find next valid target for bouncing */
    UFUNCTION(BlueprintCallable, Category = "Bouncing")
    AActor* FindBounceTarget(const FVector& CurrentLocation);
    
    /** Redirect projectile toward new target */
    UFUNCTION(BlueprintCallable, Category = "Bouncing")
    void BounceToTarget(AActor* NewTarget);
};
```

## Blueprint Integration

### Blueprint-Implementable Events

```cpp
// In projectile ability base class
UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
void OnProjectileSpawned(AActor* SpawnedProjectile);

UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
void OnProjectileImpact(AActor* Projectile, AActor* HitTarget, const FHitResult& HitResult);

UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
void OnProjectileDestroyed(AActor* Projectile);
```

### Data-Driven Configuration

```cpp
USTRUCT(BlueprintType)
struct GASCORE_API FProjectileAbilityData
{
    GENERATED_BODY()

    /** Projectile class to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    TSubclassOf<AGASCoreProjectileActor> ProjectileClass;
    
    /** Number of projectiles to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    int32 ProjectileCount = 1;
    
    /** Spread angle for multiple projectiles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float SpreadAngle = 0.f;
    
    /** Speed override */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Speed = 0.f;
    
    /** Range override */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Range = 1000.f;
    
    /** Targeting mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    EProjectileTargetingMode TargetingMode;
};
```

## Performance Optimization

### Projectile Pooling

```cpp
class GASCORE_API UProjectilePool : public USubsystem
{
    GENERATED_BODY()

public:
    /** Get pooled projectile or create new one */
    UFUNCTION(BlueprintCallable, Category = "Pool")
    AGASCoreProjectileActor* GetProjectile(TSubclassOf<AGASCoreProjectileActor> ProjectileClass);
    
    /** Return projectile to pool */
    UFUNCTION(BlueprintCallable, Category = "Pool")
    void ReturnProjectile(AGASCoreProjectileActor* Projectile);

private:
    /** Pool storage by class */
    UPROPERTY()
    TMap<TSubclassOf<AGASCoreProjectileActor>, TArray<AGASCoreProjectileActor*>> ProjectilePools;
    
    /** Maximum projectiles per pool */
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    int32 MaxPoolSize = 50;
};
```

### Culling and Cleanup

```cpp
// In projectile actor
void AGASCoreProjectileActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Set automatic cleanup timer
    if (HasAuthority())
    {
        FTimerHandle CleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [this]()
        {
            Destroy();
        }, MaxLifetime, false);
    }
}
```

## Network Considerations

### Prediction and Reconciliation

- **Client Prediction:** Spawn visual projectiles immediately on client for responsiveness
- **Server Authority:** Only server spawns authoritative projectiles with collision
- **Reconciliation:** Correct client visuals when server state differs

### Bandwidth Optimization

- **Relevancy Filtering:** Projectiles automatically culled beyond player relevance range
- **State Compression:** Use minimal replication for movement data
- **Effect Batching:** Group impact effects to reduce RPC calls

## Testing and Debugging

### Console Commands

```cpp
// Debug projectile spawning
UFUNCTION(Exec, Category = "Debug")
void SpawnDebugProjectile(float Speed = 550.f, float Range = 1000.f);

// Toggle projectile trajectory visualization
UFUNCTION(Exec, Category = "Debug")
void ToggleProjectileDebugDraw();

// Clear all projectiles
UFUNCTION(Exec, Category = "Debug")
void ClearAllProjectiles();
```

### Visual Debugging

```cpp
// In projectile ability
void UProjectileAbilityBase::DebugDrawTrajectory(const FVector& Start, const FVector& End)
{
    if (CVarProjectileDebugDraw.GetValueOnGameThread())
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f, 0, 2.f);
        DrawDebugSphere(GetWorld(), Start, 10.f, 12, FColor::Green, false, 2.f);
        DrawDebugSphere(GetWorld(), End, 15.f, 12, FColor::Blue, false, 2.f);
    }
}
```

## Common Pitfalls and Solutions

### Spawning Issues
- **Problem:** Projectiles spawn inside character geometry
- **Solution:** Use proper spawn offset and collision handling override

### Targeting Problems
- **Problem:** Projectiles miss moving targets
- **Solution:** Implement predictive targeting with target velocity

### Network Desync
- **Problem:** Client and server projectiles diverge
- **Solution:** Use server-authoritative spawning with client prediction

### Performance Impact
- **Problem:** Too many projectiles cause frame drops
- **Solution:** Implement pooling and automatic cleanup systems

## See Also

- [GASCore Projectile Actors](projectile-actors.md)
- [Gameplay Abilities Overview](../gas/abilities/overview.md)
- [Ability Tasks](../gas/abilities/ability-tasks.md)
- [Targeting Systems](../gas/abilities/targeting.md)
- [Performance Optimization](../architecture/performance.md)