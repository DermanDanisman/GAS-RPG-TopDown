# GASCore Aim Trace Task Implementation

Last updated: 2025-01-21

## Overview

The `UGASCoreTargetDataFromAimTrace` class is a specialized ability task that provides target selection through aim tracing. This task enables abilities to gather target data by performing line traces from the player's perspective or weapon, making it essential for projectile abilities, targeted spells, and precision-based gameplay mechanics.

## Class Structure

### Header Definition

```cpp
// GASCoreTargetDataFromAimTrace.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GASCoreTargetDataFromAimTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimTraceTargetDataSignature, const FHitResult&, HitResultData);

UCLASS()
class GASCORE_API UGASCoreTargetDataFromAimTrace : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GASCore|Ability Task|Target Data From Aim Trace", 
        meta = (DisplayName = "TargetDataFromAimTrace", HidePin = "OwningAbility", 
        DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
    static UGASCoreTargetDataFromAimTrace* CreateTargetDataFromAimTrace(UGameplayAbility* OwningAbility);

    UPROPERTY(BlueprintAssignable)
    FAimTraceTargetDataSignature ValidHitResultData;

private:
    virtual void Activate() override;
};
```

### Implementation

```cpp
// AGASCoreTargetDataFromAimTrace.cpp
#include "AbilitySystem/AbilityTasks/GASCoreTargetDataFromAimTrace.h"

UGASCoreTargetDataFromAimTrace* UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(
    UGameplayAbility* OwningAbility)
{
    UGASCoreTargetDataFromAimTrace* MyObj = NewAbilityTask<UGASCoreTargetDataFromAimTrace>(OwningAbility);
    return MyObj;
}

void UGASCoreTargetDataFromAimTrace::Activate()
{
    // Implementation to be added based on specific tracing requirements
}
```

## Recommended Complete Implementation

### Enhanced Header

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "GASCoreTargetDataFromAimTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimTraceTargetDataSignature, const FHitResult&, HitResultData);

/**
 * Ability task that performs aim tracing to gather target data for abilities.
 * Supports various tracing modes including camera-based and weapon-based tracing.
 */
UCLASS()
class GASCORE_API UGASCoreTargetDataFromAimTrace : public UAbilityTask
{
    GENERATED_BODY()

public:
    /**
     * Creates and returns a new aim trace task.
     * @param OwningAbility The ability that owns this task
     * @param MaxRange Maximum trace distance
     * @param TraceProfile Collision profile for trace
     * @param bUseWeaponTrace Whether to trace from weapon instead of camera
     */
    UFUNCTION(BlueprintCallable, Category = "GASCore|Ability Task|Target Data From Aim Trace", 
        meta = (DisplayName = "TargetDataFromAimTrace", HidePin = "OwningAbility", 
        DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
    static UGASCoreTargetDataFromAimTrace* CreateTargetDataFromAimTrace(
        UGameplayAbility* OwningAbility,
        float MaxRange = 1000.0f,
        FName TraceProfile = "Projectile",
        bool bUseWeaponTrace = false);

    /** Delegate fired when valid hit result is obtained */
    UPROPERTY(BlueprintAssignable)
    FAimTraceTargetDataSignature ValidHitResultData;

    /** Delegate fired when no valid target is found */
    UPROPERTY(BlueprintAssignable)
    FAimTraceTargetDataSignature NoValidTarget;

protected:
    virtual void Activate() override;

private:
    /** Maximum trace distance */
    float MaxTraceRange;
    
    /** Collision profile name for trace */
    FName CollisionProfile;
    
    /** Whether to trace from weapon instead of camera */
    bool bShouldUseWeaponTrace;
    
    /** Performs the actual line trace */
    void PerformTrace();
    
    /** Gets the trace start location based on trace mode */
    FVector GetTraceStart() const;
    
    /** Gets the trace direction based on camera or weapon orientation */
    FVector GetTraceDirection() const;
    
    /** Validates if the hit target is valid for the ability */
    bool IsValidTarget(const FHitResult& HitResult) const;
};
```

### Complete Implementation

```cpp
#include "AbilitySystem/AbilityTasks/GASCoreTargetDataFromAimTrace.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/GASCoreCombatInterface.h"

UGASCoreTargetDataFromAimTrace* UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(
    UGameplayAbility* OwningAbility, float MaxRange, FName TraceProfile, bool bUseWeaponTrace)
{
    UGASCoreTargetDataFromAimTrace* MyObj = NewAbilityTask<UGASCoreTargetDataFromAimTrace>(OwningAbility);
    MyObj->MaxTraceRange = MaxRange;
    MyObj->CollisionProfile = TraceProfile;
    MyObj->bShouldUseWeaponTrace = bUseWeaponTrace;
    return MyObj;
}

void UGASCoreTargetDataFromAimTrace::Activate()
{
    Super::Activate();
    
    // Validate task setup
    if (!Ability || !AbilitySystemComponent)
    {
        UE_LOG(LogAbilitySystem, Warning, TEXT("AimTrace task failed - missing ability or ASC"));
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            NoValidTarget.Broadcast(FHitResult());
        }
        EndTask();
        return;
    }
    
    // Perform the trace
    PerformTrace();
}

void UGASCoreTargetDataFromAimTrace::PerformTrace()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogAbilitySystem, Warning, TEXT("AimTrace task failed - no world"));
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            NoValidTarget.Broadcast(FHitResult());
        }
        EndTask();
        return;
    }
    
    // Get trace parameters
    FVector TraceStart = GetTraceStart();
    FVector TraceDirection = GetTraceDirection();
    FVector TraceEnd = TraceStart + (TraceDirection * MaxTraceRange);
    
    // Setup collision query params
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
    
    // Perform line trace
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByProfile(
        HitResult,
        TraceStart,
        TraceEnd,
        CollisionProfile,
        QueryParams
    );
    
    // Validate and broadcast result
    if (bHit && IsValidTarget(HitResult))
    {
        UE_LOG(LogAbilitySystem, Verbose, TEXT("AimTrace hit valid target: %s"), 
            *GetNameSafe(HitResult.GetActor()));
        
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            ValidHitResultData.Broadcast(HitResult);
        }
    }
    else
    {
        // No valid target found - create hit result at max range
        FHitResult NoHitResult;
        NoHitResult.Location = TraceEnd;
        NoHitResult.ImpactPoint = TraceEnd;
        NoHitResult.TraceStart = TraceStart;
        NoHitResult.TraceEnd = TraceEnd;
        
        UE_LOG(LogAbilitySystem, Verbose, TEXT("AimTrace found no valid target"));
        
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            NoValidTarget.Broadcast(NoHitResult);
        }
    }
    
    // Task completes immediately
    EndTask();
}

FVector UGASCoreTargetDataFromAimTrace::GetTraceStart() const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        return FVector::ZeroVector;
    }
    
    if (bShouldUseWeaponTrace)
    {
        // Use weapon/hand socket for trace start
        if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(AvatarActor))
        {
            return CombatInterface->GetAbilitySpawnLocation();
        }
        
        // Fall back to actor location
        return AvatarActor->GetActorLocation();
    }
    else
    {
        // Use camera for trace start
        if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
        {
            if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
            {
                FVector CameraLocation;
                FRotator CameraRotation;
                PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
                return CameraLocation;
            }
        }
        
        // Fall back to actor location + eye height
        return AvatarActor->GetActorLocation() + FVector(0, 0, 64.0f);
    }
}

FVector UGASCoreTargetDataFromAimTrace::GetTraceDirection() const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        return FVector::ForwardVector;
    }
    
    if (bShouldUseWeaponTrace)
    {
        // Use actor's forward direction for weapon trace
        return AvatarActor->GetActorForwardVector();
    }
    else
    {
        // Use camera direction for camera trace
        if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
        {
            if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
            {
                FVector CameraLocation;
                FRotator CameraRotation;
                PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
                return CameraRotation.Vector();
            }
        }
        
        // Fall back to actor forward
        return AvatarActor->GetActorForwardVector();
    }
}

bool UGASCoreTargetDataFromAimTrace::IsValidTarget(const FHitResult& HitResult) const
{
    // Basic validation
    if (!HitResult.bBlockingHit || !HitResult.GetActor())
    {
        return false;
    }
    
    AActor* HitActor = HitResult.GetActor();
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    
    // Don't target self
    if (HitActor == AvatarActor)
    {
        return false;
    }
    
    // Additional validation can be added here:
    // - Team affiliation checks
    // - Combat interface validation
    // - Ability-specific target requirements
    
    return true;
}
```

## Usage Examples

### Basic Usage in Abilities

```cpp
// In your projectile ability
void UMyProjectileAbility::ActivateAbility(/*...*/)
{
    // Create aim trace task
    UGASCoreTargetDataFromAimTrace* AimTask = 
        UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(
            this, 
            1500.0f,        // Max range
            "Projectile",   // Collision profile
            false           // Use camera trace
        );
    
    // Bind delegates
    AimTask->ValidHitResultData.AddDynamic(this, &UMyProjectileAbility::OnTargetAcquired);
    AimTask->NoValidTarget.AddDynamic(this, &UMyProjectileAbility::OnNoTarget);
    
    // Activate task
    AimTask->ReadyForActivation();
}

void UMyProjectileAbility::OnTargetAcquired(const FHitResult& HitResult)
{
    // Use hit result for projectile targeting
    TargetLocation = HitResult.ImpactPoint;
    
    if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        SpawnProjectileToTarget(TargetLocation);
    }
    
    K2_EndAbility();
}

void UMyProjectileAbility::OnNoTarget(const FHitResult& HitResult)
{
    // Still spawn projectile toward trace end point
    TargetLocation = HitResult.Location;
    
    if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        SpawnProjectileToTarget(TargetLocation);
    }
    
    K2_EndAbility();
}
```

### Blueprint Usage

In Blueprint, the task appears as a node with multiple outputs:

```
[Event ActivateAbility]
        ↓
[Target Data From Aim Trace]
    ├─Exec Out─→ [Continue other logic]
    ├─Valid Hit Result Data─→ [Process Target] → [Spawn Projectile] → [End Ability]
    └─No Valid Target─→ [Default Behavior] → [End Ability]
```

### Advanced Usage with Custom Parameters

```cpp
// Enhanced usage with custom configuration
void UAdvancedTargetingAbility::ActivateAbility(/*...*/)
{
    // Different trace parameters based on ability level
    float TraceRange = 1000.0f + (GetAbilityLevel() * 200.0f);
    FName ProfileName = IsRangedWeapon() ? "RangedWeapon" : "MeleeWeapon";
    bool bUseWeapon = ShouldUseWeaponTrace();
    
    UGASCoreTargetDataFromAimTrace* AimTask = 
        UGASCoreTargetDataFromAimTrace::CreateTargetDataFromAimTrace(
            this, TraceRange, ProfileName, bUseWeapon);
    
    AimTask->ValidHitResultData.AddDynamic(this, &UAdvancedTargetingAbility::OnTargetFound);
    AimTask->NoValidTarget.AddDynamic(this, &UAdvancedTargetingAbility::OnTargetMissed);
    
    AimTask->ReadyForActivation();
}
```

## Integration with Other Systems

### Weapon System Integration

```cpp
FVector UGASCoreTargetDataFromAimTrace::GetWeaponTraceStart() const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return FVector::ZeroVector;
    
    // Try to get weapon component
    if (UActorComponent* WeaponComp = AvatarActor->GetComponentByClass(UWeaponComponent::StaticClass()))
    {
        if (UWeaponComponent* Weapon = Cast<UWeaponComponent>(WeaponComp))
        {
            return Weapon->GetMuzzleLocation();
        }
    }
    
    // Fall back to combat interface
    if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(AvatarActor))
    {
        return CombatInterface->GetAbilitySpawnLocation();
    }
    
    return AvatarActor->GetActorLocation();
}
```

### Team System Integration

```cpp
bool UGASCoreTargetDataFromAimTrace::IsValidTarget(const FHitResult& HitResult) const
{
    if (!HitResult.bBlockingHit || !HitResult.GetActor())
        return false;
    
    AActor* HitActor = HitResult.GetActor();
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    
    // Team-based validation
    if (ITeamInterface* HitTeam = Cast<ITeamInterface>(HitActor))
    {
        if (ITeamInterface* AvatarTeam = Cast<ITeamInterface>(AvatarActor))
        {
            // Don't target allies
            if (HitTeam->GetTeamID() == AvatarTeam->GetTeamID())
            {
                return false;
            }
        }
    }
    
    // Must have ability system component to be a valid target
    if (!Cast<IAbilitySystemInterface>(HitActor))
    {
        return false;
    }
    
    return true;
}
```

## Debugging and Visualization

### Debug Drawing

```cpp
void UGASCoreTargetDataFromAimTrace::PerformTrace()
{
    // ... existing trace code ...
    
#if !UE_BUILD_SHIPPING
    if (CVarDebugAimTrace.GetValueOnGameThread())
    {
        // Draw trace line
        FColor TraceColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, 2.0f, 0, 2.0f);
        
        if (bHit)
        {
            // Draw hit point
            DrawDebugSphere(World, HitResult.ImpactPoint, 10.0f, 12, FColor::Yellow, false, 2.0f);
            
            // Draw hit normal
            DrawDebugDirectionalArrow(World, HitResult.ImpactPoint, 
                HitResult.ImpactPoint + HitResult.ImpactNormal * 50.0f, 
                20.0f, FColor::Blue, false, 2.0f);
        }
    }
#endif
    
    // ... rest of method ...
}
```

### Console Commands

```cpp
// Add to your game module
static TAutoConsoleVariable<bool> CVarDebugAimTrace(
    TEXT("gas.debug.aimtrace"),
    false,
    TEXT("Enable aim trace debug visualization"));

static TAutoConsoleVariable<bool> CVarLogAimTrace(
    TEXT("gas.log.aimtrace"),
    false,
    TEXT("Log aim trace results"));
```

### Logging

```cpp
void UGASCoreTargetDataFromAimTrace::PerformTrace()
{
    UE_CLOG(CVarLogAimTrace.GetValueOnGameThread(), LogAbilitySystem, Log, 
        TEXT("AimTrace: Start=%s, End=%s, Profile=%s"), 
        *TraceStart.ToString(), *TraceEnd.ToString(), *CollisionProfile.ToString());
    
    // ... trace code ...
    
    if (bHit)
    {
        UE_CLOG(CVarLogAimTrace.GetValueOnGameThread(), LogAbilitySystem, Log, 
            TEXT("AimTrace Hit: Actor=%s, Location=%s, Distance=%.2f"),
            *GetNameSafe(HitResult.GetActor()), 
            *HitResult.ImpactPoint.ToString(),
            HitResult.Distance);
    }
}
```

## Performance Considerations

### Trace Optimization

```cpp
void UGASCoreTargetDataFromAimTrace::PerformTrace()
{
    // Use simpler collision for performance
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;  // Use simple collision
    QueryParams.bReturnPhysicalMaterial = false;  // Don't need material info
    
    // Limit trace to specific object types
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
    
    bool bHit = World->LineTraceSingleByObjectType(
        HitResult, TraceStart, TraceEnd, ObjectParams, QueryParams);
    
    // ... rest of method ...
}
```

### Async Tracing (Advanced)

```cpp
// For very long-range traces or multiple traces, consider async tracing
void UGASCoreTargetDataFromAimTrace::PerformAsyncTrace()
{
    FTraceDelegate TraceDelegate;
    TraceDelegate.BindUObject(this, &UGASCoreTargetDataFromAimTrace::OnTraceComplete);
    
    GetWorld()->AsyncLineTraceByProfile(
        EAsyncTraceType::Single,
        TraceStart,
        TraceEnd,
        CollisionProfile,
        QueryParams,
        &TraceDelegate
    );
}

void UGASCoreTargetDataFromAimTrace::OnTraceComplete(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum)
{
    if (TraceDatum.OutHits.Num() > 0)
    {
        FHitResult& HitResult = TraceDatum.OutHits[0];
        if (IsValidTarget(HitResult))
        {
            ValidHitResultData.Broadcast(HitResult);
        }
        else
        {
            NoValidTarget.Broadcast(HitResult);
        }
    }
    else
    {
        FHitResult NoHit;
        NoHit.Location = TraceEnd;
        NoValidTarget.Broadcast(NoHit);
    }
    
    EndTask();
}
```

## Related Documentation

- [Gameplay Ability Tasks](gameplay-ability-tasks.md)
- [Spawn Projectile System](spawn-projectile.md)
- [Projectile Abilities Pattern](projectile-abilities.md)
- [Combat Interface](combat-interface.md)
- [Gameplay Abilities Concepts and Practice](../guides/GameplayAbilities_Concepts_And_Practice.md)

## See Also

- [Unreal Engine Line Tracing](https://docs.unrealengine.com/5.3/en-US/line-traces-in-unreal-engine/)
- [Unreal Engine Collision Profiles](https://docs.unrealengine.com/5.3/en-US/collision-filtering-in-unreal-engine/)
- [Gameplay Ability System Tasks](https://docs.unrealengine.com/5.3/en-US/ability-tasks-in-unreal-engine/)