# Projectile System Implementation Guide

Last updated: 2024-12-19

## Quick Start

This guide provides a step-by-step walkthrough for implementing projectile-based abilities using the GAS-RPG-TopDown projectile system.

## Prerequisites

- Basic understanding of Unreal Engine's Gameplay Ability System (GAS)
- Familiarity with C++ and Blueprint development in Unreal Engine
- Understanding of actor spawning and collision systems

## Step 1: Create a Custom Projectile Actor

### Inherit from GASCoreProjectileActor

```cpp
// MyProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "Actors/GASCoreProjectileActor.h"
#include "MyProjectile.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MYGAME_API AMyProjectile : public AGASCoreProjectileActor
{
    GENERATED_BODY()

public:
    AMyProjectile();

protected:
    virtual void BeginPlay() override;
    virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult) override;

    /** Gameplay effect to apply on impact */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> DamageEffect;

    /** Visual/audio effects for impact */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TObjectPtr<UNiagaraSystem> ImpactEffect;

    /** Validate if target can be damaged */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual bool IsValidTarget(AActor* Target) const;

    /** Apply damage effect to target */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void ApplyDamageToTarget(AActor* Target);

    /** Spawn impact effects and sounds */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    virtual void HandleImpact(const FHitResult& HitResult);
};
```

### Implement Collision Logic

```cpp
// MyProjectile.cpp
#include "MyProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AMyProjectile::AMyProjectile()
{
    // Additional projectile configuration if needed
}

void AMyProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    // Additional initialization
}

void AMyProjectile::OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereCollisionOverlap(OverlappedComponent, OtherActor, OtherComp, 
        OtherBodyIndex, bFromSweep, SweepResult);

    // Validate target
    if (!IsValidTarget(OtherActor))
        return;

    // Apply damage
    ApplyDamageToTarget(OtherActor);

    // Handle impact effects
    HandleImpact(SweepResult);

    // Destroy projectile
    Destroy();
}

bool AMyProjectile::IsValidTarget(AActor* Target) const
{
    if (!Target || Target == GetOwner() || Target == GetInstigator())
        return false;

    // Check if target has ability system component
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    return TargetASC != nullptr;
}

void AMyProjectile::ApplyDamageToTarget(AActor* Target)
{
    if (!DamageEffect)
        return;

    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!TargetASC)
        return;

    // Create effect context
    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);
    EffectContext.AddInstigator(GetInstigator(), this);

    // Create and apply effect spec
    FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(
        DamageEffect, 1.0f, EffectContext);

    if (EffectSpec.IsValid())
    {
        TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
    }
}

void AMyProjectile::HandleImpact(const FHitResult& HitResult)
{
    // Spawn impact effects
    if (ImpactEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
    }

    // Play impact sound
    // Additional impact logic...
}
```

## Step 2: Create a Projectile Ability

### Define the Ability Class

```cpp
// MyProjectileAbility.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GASCoreGameplayAbility.h"
#include "MyProjectileAbility.generated.h"

UENUM(BlueprintType)
enum class EMyTargetingMode : uint8
{
    CursorLocation,
    ClosestEnemy,
    ForwardDirection
};

UCLASS(BlueprintType, Blueprintable)
class MYGAME_API UMyProjectileAbility : public UGASCoreGameplayAbility
{
    GENERATED_BODY()

public:
    UMyProjectileAbility();

protected:
    // Ability implementation
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /** Projectile class to spawn */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TSubclassOf<class AMyProjectile> ProjectileClass;

    /** Targeting mode for the projectile */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    EMyTargetingMode TargetingMode = EMyTargetingMode::CursorLocation;

    /** Maximum range for the projectile */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float MaxRange = 1500.f;

    /** Offset from character for spawning */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    FVector SpawnOffset = FVector(100.f, 0.f, 50.f);

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual FVector CalculateSpawnLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual FVector CalculateTargetLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual AActor* SpawnProjectile(const FVector& StartLocation, const FVector& TargetLocation);

    // Validation
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Projectile")
    virtual bool CanActivateProjectileAbility() const;
};
```

### Implement the Ability Logic

```cpp
// MyProjectileAbility.cpp
#include "MyProjectileAbility.h"
#include "MyProjectile.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

UMyProjectileAbility::UMyProjectileAbility()
{
    // Configure ability tags
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Offensive.Projectile"));
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Casting"));
    
    // Set default projectile class
    ProjectileClass = AMyProjectile::StaticClass();
}

void UMyProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // Validate ability can activate
    if (!CanActivateProjectileAbility())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Commit ability (cost and cooldown)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Calculate spawn and target locations
    FVector SpawnLocation = CalculateSpawnLocation();
    FVector TargetLocation = CalculateTargetLocation();

    // Spawn projectile on server
    if (HasAuthority(&ActivationInfo))
    {
        AActor* SpawnedProjectile = SpawnProjectile(SpawnLocation, TargetLocation);
        if (!SpawnedProjectile)
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
            return;
        }
    }

    // End ability immediately (projectile handles the rest)
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

FVector UMyProjectileAbility::CalculateSpawnLocation() const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
        return FVector::ZeroVector;

    FVector ActorLocation = AvatarActor->GetActorLocation();
    FVector ActorForward = AvatarActor->GetActorForwardVector();
    FVector ActorRight = AvatarActor->GetActorRightVector();
    FVector ActorUp = AvatarActor->GetActorUpVector();

    // Apply spawn offset relative to actor's orientation
    return ActorLocation + 
           (ActorForward * SpawnOffset.X) + 
           (ActorRight * SpawnOffset.Y) + 
           (ActorUp * SpawnOffset.Z);
}

FVector UMyProjectileAbility::CalculateTargetLocation() const
{
    switch (TargetingMode)
    {
        case EMyTargetingMode::CursorLocation:
        {
            // Get cursor world position
            APlayerController* PC = GetActorInfo().PlayerController.Get();
            if (!PC) return FVector::ZeroVector;

            FHitResult HitResult;
            if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
            {
                return HitResult.Location;
            }
            return FVector::ZeroVector;
        }

        case EMyTargetingMode::ForwardDirection:
        {
            AActor* AvatarActor = GetAvatarActorFromActorInfo();
            if (!AvatarActor) return FVector::ZeroVector;

            FVector StartLocation = CalculateSpawnLocation();
            FVector ForwardDirection = AvatarActor->GetActorForwardVector();
            return StartLocation + (ForwardDirection * MaxRange);
        }

        case EMyTargetingMode::ClosestEnemy:
        {
            // Implement closest enemy finding logic
            // This is a simplified version
            AActor* AvatarActor = GetAvatarActorFromActorInfo();
            if (!AvatarActor) return FVector::ZeroVector;

            // Find closest enemy within range
            // Implementation depends on your enemy detection system
            return FVector::ZeroVector;
        }

        default:
            return FVector::ZeroVector;
    }
}

AActor* UMyProjectileAbility::SpawnProjectile(const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!ProjectileClass)
        return nullptr;

    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    // Calculate direction and rotation
    FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
    FRotator SpawnRotation = Direction.Rotation();

    // Configure spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetAvatarActorFromActorInfo();
    SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn projectile
    AActor* SpawnedProjectile = World->SpawnActor<AActor>(
        ProjectileClass, StartLocation, SpawnRotation, SpawnParams);

    return SpawnedProjectile;
}

bool UMyProjectileAbility::CanActivateProjectileAbility() const
{
    // Check if projectile class is valid
    if (!ProjectileClass)
        return false;

    // Check if avatar actor exists
    if (!GetAvatarActorFromActorInfo())
        return false;

    // Additional validation logic
    return true;
}
```

## Step 3: Configure Character Integration

### Ensure Character Implements Required Interfaces

Your character should inherit from `ATDCharacterBase` or implement the necessary interfaces:

```cpp
// MyCharacter.h
#include "Charcters/TDCharacterBase.h"

UCLASS()
class MYGAME_API AMyCharacter : public ATDCharacterBase
{
    GENERATED_BODY()

public:
    AMyCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void InitializeAbilityActorInfo() override;
};
```

### Add Ability to Startup Abilities

In your character's ability initialization component:

```cpp
// In character Blueprint or C++
void AMyCharacter::SetupStartupAbilities()
{
    if (AbilityInitComponent)
    {
        // Add your projectile ability to startup abilities
        TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
        StartupAbilities.Add(UMyProjectileAbility::StaticClass());
        
        AbilityInitComponent->SetStartupAbilities(StartupAbilities);
    }
}
```

## Step 4: Create Gameplay Effects

### Damage Effect

Create a `UGameplayEffect` for damage application:

```cpp
// In Blueprint or C++
// DamageEffect_Base (Instant Gameplay Effect)
// - Duration: Instant
// - Modifiers:
//   - Attribute: Health
//   - Operation: Add
//   - Magnitude: -25 (or use MMC for level scaling)
```

### Cost and Cooldown Effects

```cpp
// ManaCost_Projectile (Instant)
// - Modifiers:
//   - Attribute: Mana
//   - Operation: Add  
//   - Magnitude: -20

// Cooldown_Projectile (Duration: 3 seconds)
// - Grant Tags: Cooldown.Ability.Projectile
```

## Step 5: Setup Input Binding

### Enhanced Input Configuration

```cpp
// In your input configuration
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UTDEnhancedInputComponent* TDInputComponent = Cast<UTDEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind projectile ability to input
        TDInputComponent->BindAbilityActions(InputConfig, this,
            &ThisClass::AbilityInputTagPressed,
            &ThisClass::AbilityInputTagReleased, 
            &ThisClass::AbilityInputTagHeld);
    }
}

void AMyCharacter::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // Try to activate ability with matching input tag
        ASC->AbilityInputTagPressed(InputTag);
    }
}
```

## Step 6: Blueprint Configuration

### Create Blueprint Classes

1. **Create Projectile Blueprint**
   - Inherit from your C++ projectile class (`AMyProjectile`)
   - Add visual mesh/effects
   - Configure damage effect class
   - Set up impact effects

2. **Create Ability Blueprint**
   - Inherit from your C++ ability class (`UMyProjectileAbility`)
   - Set projectile class reference
   - Configure targeting mode
   - Set up cost/cooldown effects

3. **Configure Character Blueprint**
   - Add ability to startup abilities array
   - Set up input bindings
   - Configure ability system component

## Step 7: Testing and Debugging

### Test in Editor

1. **Basic Functionality Test**
   ```
   - PIE (Play in Editor)
   - Press ability input key
   - Verify projectile spawns
   - Verify projectile moves toward target
   - Verify collision detection works
   ```

2. **Network Test**
   ```
   - PIE with multiple clients
   - Test ability activation on client
   - Verify server authority for damage
   - Check projectile replication
   ```

### Debug Console Commands

```cpp
// Add these to your character or game mode
UFUNCTION(Exec, Category = "Debug")
void DebugSpawnProjectile();

UFUNCTION(Exec, Category = "Debug") 
void ToggleProjectileDebugDraw();

UFUNCTION(Exec, Category = "Debug")
void ListActiveProjectiles();
```

## Common Issues and Solutions

### Issue: Projectile Spawns Inside Character

**Solution:** Increase spawn offset or use character bounds calculation:

```cpp
FVector UMyProjectileAbility::CalculateSpawnLocation() const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return FVector::ZeroVector;

    // Use character bounds to avoid spawning inside mesh
    FVector Origin, BoxExtent;
    AvatarActor->GetActorBounds(false, Origin, BoxExtent);
    
    FVector ForwardOffset = AvatarActor->GetActorForwardVector() * (BoxExtent.X + 50.f);
    return Origin + ForwardOffset + FVector(0, 0, 50.f);
}
```

### Issue: Projectile Doesn't Hit Targets

**Solution:** Check collision configuration:

```cpp
// In projectile constructor
SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
SphereCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
```

### Issue: Ability Doesn't Activate

**Solution:** Verify ability granting and input setup:

```cpp
// Debug ability system state
void AMyCharacter::DebugAbilitySystem()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        UE_LOG(LogTemp, Warning, TEXT("Granted Abilities: %d"), ASC->GetActivatableAbilities().Num());
        
        for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
        {
            UE_LOG(LogTemp, Warning, TEXT("Ability: %s"), 
                Spec.Ability ? *Spec.Ability->GetName() : TEXT("Null"));
        }
    }
}
```

## Performance Optimization

### Object Pooling

```cpp
// Implement projectile pooling for frequently spawned projectiles
class UProjectilePool : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Pool")
    AMyProjectile* GetPooledProjectile(TSubclassOf<AMyProjectile> ProjectileClass);
    
    UFUNCTION(BlueprintCallable, Category = "Pool")
    void ReturnProjectileToPool(AMyProjectile* Projectile);

private:
    UPROPERTY()
    TMap<TSubclassOf<AMyProjectile>, TArray<AMyProjectile*>> ProjectilePools;
};
```

### Automatic Cleanup

```cpp
// In projectile BeginPlay
void AMyProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-destroy after 10 seconds if not hit anything
    SetLifeSpan(10.f);
}
```

## Next Steps

1. **Implement Advanced Features**
   - Homing projectiles
   - Area-of-effect projectiles
   - Bouncing projectiles
   - Chain projectiles

2. **Visual Polish**
   - Trail effects using Niagara
   - Impact effects and sound
   - Projectile deformation/scaling

3. **Gameplay Features**
   - Projectile interception
   - Deflection/reflection
   - Elemental damage types

## See Also

- [Projectile System Overview](../systems/projectile-system-overview.md)
- [GASCore Projectile Actors](../plugin/projectile-actors.md)
- [Projectile Abilities Pattern](../plugin/projectile-abilities.md)
- [GAS Abilities Overview](../gas/abilities/overview.md)
- [Character Base Implementation](../architecture/character-base.md)