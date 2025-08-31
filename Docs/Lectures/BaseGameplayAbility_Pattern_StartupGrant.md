# Base Gameplay Ability + Startup Abilities Granting Pattern (Deep Dive)

This comprehensive guide explores the architectural patterns for creating a robust base gameplay ability class and implementing automatic startup ability granting in your GAS-powered game. These patterns form the foundation for maintainable, scalable ability systems.

## Table of Contents

- [Why a Base Ability Class?](#why-a-base-ability-class)
- [Creating the Base Class (UTDGameplayAbilityBase)](#creating-the-base-class-utdgameplayabilitybase)
- [Startup Abilities on Character](#startup-abilities-on-character)
- [Server-Authoritative Granting](#server-authoritative-granting)
- [Implementation Examples](#implementation-examples)
- [Advanced Patterns](#advanced-patterns)
- [Integration with Project Architecture](#integration-with-project-architecture)
- [Common Pitfalls](#common-pitfalls)
- [Testing and Validation](#testing-and-validation)
- [Best Practices Checklist](#best-practices-checklist)

## Why a Base Ability Class?

Creating a base ability class provides a centralized foundation for shared behavior across all abilities in your project. This pattern offers several critical advantages for maintainable game development.

### Centralized Shared Behavior

A base class eliminates code duplication by providing common functionality that all abilities need:

- **Consistent Instancing Policy**: Set default instancing behavior for all abilities
- **Standard Input Tag Handling**: Unified approach to input binding and activation
- **Common Hook Points**: Standardized places for analytics, debugging, and effects
- **Cost/Cooldown Conventions**: Consistent patterns for resource management

### Future-Proofing Your Architecture

Base classes make it easy to add new features to all abilities simultaneously:

```cpp
// Future enhancement example: Adding analytics to all abilities
class UTDGameplayAbilityBase : public UGameplayAbility
{
    virtual void ActivateAbility(/*...*/) override
    {
        // All abilities now automatically track usage
        RecordAbilityUsageAnalytics();
        Super::ActivateAbility(/*...*/);
    }

    virtual void EndAbility(/*...*/) override
    {
        // All abilities now track completion
        RecordAbilityCompletionAnalytics();
        Super::EndAbility(/*...*/);
    }
};
```

### Consistent Configuration

Base classes ensure all team members follow the same patterns and conventions:

```cpp
// Standard configuration applied to all abilities
UTDGameplayAbilityBase::UTDGameplayAbilityBase()
{
    // Consistent instancing policy
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    // Standard network execution for responsive gameplay
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    
    // Default replication policy
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
    
    // Standard timing for ability system integration
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}
```

### Team Development Benefits

- **Onboarding**: New developers learn one pattern that applies everywhere
- **Code Review**: Reviewers can focus on ability-specific logic rather than boilerplate
- **Debugging**: Consistent logging and debug information across all abilities
- **Maintenance**: Updates and fixes can be applied to all abilities at once

## Creating the Base Class (UTDGameplayAbilityBase)

Let's design a robust base ability class that will serve as the foundation for all abilities in the TD RPG TopDown project.

### Minimal Starting Implementation

Since the project doesn't currently have a base gameplay ability class, here's what the initial implementation would look like:

#### Header File (UTDGameplayAbilityBase.h)

```cpp
// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, 
// or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TDGameplayAbilityBase.generated.h"

/**
 * UTDGameplayAbilityBase
 * 
 * Base class for all gameplay abilities in the TD RPG TopDown project.
 * Provides common functionality, consistent configuration, and shared behavior patterns.
 * 
 * Design Philosophy:
 * - Start minimal, extend as project needs grow
 * - Provide sensible defaults for network, instancing, and execution policies
 * - Centralize common patterns like logging, analytics, and debug information
 * - Maintain compatibility with both C++ and Blueprint ability implementations
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class RPG_TOPDOWN_API UTDGameplayAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UTDGameplayAbilityBase();

    // ===== Core Lifecycle Overrides =====
    
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    // ===== Project-Specific Utilities =====

    /** Get the TD-specific ASC from the actor info */
    UFUNCTION(BlueprintCallable, Category = "TD|Ability")
    class UTDAbilitySystemComponent* GetTDAbilitySystemComponent() const;

    /** Get the owning character if this ability is owned by one */
    UFUNCTION(BlueprintCallable, Category = "TD|Ability")
    class ATDCharacterBase* GetTDCharacterFromActorInfo() const;

protected:
    // ===== Debugging and Analytics =====

    /** Whether this ability should log detailed activation information */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bEnableDebugLogging = false;

    /** Ability description for debugging and designer reference */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (MultiLine = true))
    FString AbilityDescription;

    // ===== Future Extension Points =====

    /** Called when ability starts - override for common startup behavior */
    UFUNCTION(BlueprintImplementableEvent, Category = "TD|Ability")
    void OnAbilityStart();

    /** Called when ability ends - override for common cleanup behavior */
    UFUNCTION(BlueprintImplementableEvent, Category = "TD|Ability")
    void OnAbilityEnd(bool bWasCancelled);

private:
    /** Internal logging helper */
    void LogAbilityAction(const FString& Action, const FGameplayAbilityActorInfo* ActorInfo) const;
};
```

#### Implementation File (UTDGameplayAbilityBase.cpp)

```cpp
#include "AbilitySystem/Abilities/TDGameplayAbilityBase.h"
#include "AbilitySystem/Components/TDAbilitySystemComponent.h"
#include "Characters/TDCharacterBase.h"

UTDGameplayAbilityBase::UTDGameplayAbilityBase()
{
    // ===== Default Policies for Project Consistency =====
    
    // Allow multiple instances per actor for complex ability combinations
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    // Enable client prediction for responsive multiplayer gameplay
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    
    // Replicate ability state for network games
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
    
    // Allow both client and server to initiate (with prediction)
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UTDGameplayAbilityBase::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // Log activation for debugging
    LogAbilityAction(TEXT("ACTIVATED"), ActorInfo);

    // Call Blueprint event for designer extensibility
    OnAbilityStart();

    // Always call super to maintain GAS functionality
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UTDGameplayAbilityBase::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // Log ending for debugging
    LogAbilityAction(bWasCancelled ? TEXT("CANCELLED") : TEXT("ENDED"), ActorInfo);

    // Call Blueprint event for designer extensibility
    OnAbilityEnd(bWasCancelled);

    // Always call super to maintain GAS functionality
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UTDAbilitySystemComponent* UTDGameplayAbilityBase::GetTDAbilitySystemComponent() const
{
    return Cast<UTDAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

ATDCharacterBase* UTDGameplayAbilityBase::GetTDCharacterFromActorInfo() const
{
    if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
    {
        return Cast<ATDCharacterBase>(ActorInfo->AvatarActor.Get());
    }
    return nullptr;
}

void UTDGameplayAbilityBase::LogAbilityAction(const FString& Action, const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (bEnableDebugLogging)
    {
        const FString ActorName = ActorInfo && ActorInfo->AvatarActor.IsValid() 
            ? ActorInfo->AvatarActor->GetName() 
            : TEXT("Unknown");

        const FString AuthorityInfo = HasAuthority(ActorInfo) ? TEXT("SERVER") : TEXT("CLIENT");

        UE_LOG(LogTemp, Log, TEXT("[%s] %s %s: %s"), 
               *AuthorityInfo, *Action, *GetClass()->GetName(), *ActorName);
               
        if (GEngine && bEnableDebugLogging)
        {
            FColor DebugColor = HasAuthority(ActorInfo) ? FColor::Green : FColor::Orange;
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, DebugColor,
                FString::Printf(TEXT("[%s] %s %s"), 
                               *AuthorityInfo, *Action, *GetClass()->GetName()));
        }
    }
}
```

### Suggested Default Configuration

The base class establishes sensible defaults that work well for most abilities:

**Instancing Policy**: `InstancedPerActor` allows multiple abilities to run simultaneously, which is essential for complex gameplay systems.

**Net Execution Policy**: `LocalPredicted` provides responsive multiplayer gameplay while maintaining server authority.

**Replication Policy**: `ReplicateYes` ensures ability state is properly synchronized in networked games.

### Blueprint Integration

Blueprint designers can inherit from the base class and gain all the benefits:

1. **Create New Blueprint**: Choose "TD Gameplay Ability Base" as parent class
2. **Override Events**: Use `OnAbilityStart` and `OnAbilityEnd` events for custom logic
3. **Access Utilities**: Use the provided helper functions like `GetTDCharacterFromActorInfo`

## Startup Abilities on Character

The startup abilities pattern automatically grants essential abilities to characters when they're initialized. This system ensures characters have their core abilities available without manual intervention.

### Character-Level Configuration

Characters should expose an editable array of abilities that are granted automatically during initialization:

#### ATDCharacterBase Enhancement

```cpp
// Add to ATDCharacterBase.h
UCLASS(Abstract)
class RPG_TOPDOWN_API ATDCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    // ... existing code ...

protected:
    /** Abilities automatically granted to this character on initialization */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities",
              meta = (AllowPrivateAccess = "true"))
    TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

    /** Level to grant startup abilities at (1 by default) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities",
              meta = (ClampMin = 1, ClampMax = 100))
    int32 StartupAbilityLevel = 1;

    /** Whether to grant startup abilities automatically during InitializeAbilityActorInfo */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    bool bAutoGrantStartupAbilities = true;

    // ... rest of class ...

public:
    /** Manually grant all startup abilities (useful for delayed granting scenarios) */
    UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
    virtual void GrantStartupAbilities();

    /** Remove all startup abilities (useful for testing or special scenarios) */
    UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
    virtual void RemoveStartupAbilities();
};
```

#### Implementation in ATDCharacterBase

```cpp
// Add to ATDCharacterBase.cpp
void ATDCharacterBase::InitializeAbilityActorInfo()
{
    // ... existing initialization code ...

    // Grant startup abilities after ASC is properly initialized
    if (bAutoGrantStartupAbilities && HasAuthority())
    {
        GrantStartupAbilities();
    }
}

void ATDCharacterBase::GrantStartupAbilities()
{
    // Only the server should grant abilities
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, 
               TEXT("GrantStartupAbilities called on non-authority. Abilities must be granted by server."));
        return;
    }

    if (!IsValid(AbilitySystemComponent))
    {
        UE_LOG(LogTemp, Warning, 
               TEXT("Cannot grant startup abilities: AbilitySystemComponent is invalid"));
        return;
    }

    // Grant each startup ability
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
    {
        if (IsValid(AbilityClass))
        {
            FGameplayAbilitySpec AbilitySpec(
                AbilityClass,           // Ability class
                StartupAbilityLevel,    // Level
                INDEX_NONE,             // Input ID (can be set later for input-bound abilities)
                this                    // Source object
            );

            AbilitySystemComponent->GiveAbility(AbilitySpec);

            UE_LOG(LogTemp, Log, 
                   TEXT("Granted startup ability: %s to %s at level %d"),
                   *AbilityClass->GetName(), *GetName(), StartupAbilityLevel);
        }
        else
        {
            UE_LOG(LogTemp, Warning, 
                   TEXT("Null ability class in StartupAbilities array for %s"), *GetName());
        }
    }
}

void ATDCharacterBase::RemoveStartupAbilities()
{
    if (!HasAuthority() || !IsValid(AbilitySystemComponent))
    {
        return;
    }

    // Find and remove startup abilities
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
    {
        if (IsValid(AbilityClass))
        {
            AbilitySystemComponent->ClearAbility(
                AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass));
        }
    }
}
```

### Blueprint Configuration

Designers can easily configure startup abilities through the Blueprint editor:

1. **Select Character Blueprint**: Open ATDPlayerCharacter_BP or similar
2. **Find Startup Abilities**: Locate the "GAS|Abilities" category in details panel
3. **Add Abilities**: Use the "+" button to add ability classes to the array
4. **Configure Level**: Set the level at which abilities should be granted
5. **Test Configuration**: The abilities will be automatically granted during play

### Hierarchical Ability Granting

Different character types can have different default abilities:

```cpp
// Base character: Movement abilities
ATDCharacterBase::StartupAbilities = {
    UTDAbility_Walk::StaticClass(),
    UTDAbility_Jump::StaticClass()
};

// Player character: Base + player-specific abilities  
ATDPlayerCharacter::StartupAbilities = {
    UTDAbility_Walk::StaticClass(),
    UTDAbility_Jump::StaticClass(),
    UTDAbility_BasicAttack::StaticClass(),
    UTDAbility_Inventory::StaticClass()
};

// Enemy character: Base + AI-specific abilities
ATDEnemyCharacter::StartupAbilities = {
    UTDAbility_Walk::StaticClass(),
    UTDAbility_AIAttack::StaticClass(),
    UTDAbility_AIAlert::StaticClass()
};
```

## Server-Authoritative Granting

Security and consistency in multiplayer games require that ability granting follows strict server authority patterns. This section covers the implementation details and best practices.

### Authority Validation

All ability granting must be validated by the server:

```cpp
void ATDCharacterBase::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    // CRITICAL: Only server can grant abilities
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, 
               TEXT("Attempted to grant ability %s on non-authority! This is a security violation."),
               *AbilityClass->GetName());
        return;
    }

    // Validate inputs
    if (!IsValid(AbilityClass) || !IsValid(AbilitySystemComponent))
    {
        return;
    }

    // Server authoritative granting
    FGameplayAbilitySpec NewAbilitySpec(AbilityClass, Level, INDEX_NONE, this);
    FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

    // Log for audit trail
    UE_LOG(LogTemp, Log, 
           TEXT("SERVER: Granted ability %s to %s (Handle: %s)"),
           *AbilityClass->GetName(), *GetName(), *Handle.ToString());
}
```

### Automatic Replication

Once granted on the server, ability specs automatically replicate to the owning client:

```cpp
// The replication happens automatically, but you can monitor it:
void ATDPlayerState::NotifyActorOnInputTouchBegin(const ETouchIndex::Type FingerIndex, const FVector Location)
{
    Super::NotifyActorOnInputTouchBegin(FingerIndex, Location);

    // On clients, log when ability specs are received
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Log, 
               TEXT("CLIENT: Received %d ability specs from server"),
               AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
                   FGameplayTagContainer(), false).Num());
    }
}
```

### Timing Considerations

Ability granting must happen at the correct time in the initialization sequence:

#### For Player Characters

```cpp
void ATDPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Initialize ASC after possession
    InitializeAbilityActorInfo();
    
    // Startup abilities are granted in InitializeAbilityActorInfo
    // This ensures PlayerState and ASC are properly set up first
}

void ATDPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // Initialize on client when PlayerState replicates
    InitializeAbilityActorInfo();
    
    // Client will receive replicated ability specs automatically
}
```

#### For AI Characters

```cpp
void ATDEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // AI characters can initialize immediately
    InitializeAbilityActorInfo();
    
    // Startup abilities granted if HasAuthority() is true
}
```

### Security Considerations

Prevent client-side ability granting exploits:

```cpp
// ❌ DANGEROUS: Client-accessible ability granting
UFUNCTION(BlueprintCallable, CallInEditor = true) // Accessible to clients!
void UnsafeGrantAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
    // Clients could call this to grant themselves any ability!
    GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
}

// ✅ SECURE: Server-only ability granting
UFUNCTION(BlueprintCallable, CallInEditor = true, BlueprintAuthorityOnly)
void SecureGrantAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
    // BlueprintAuthorityOnly ensures this only runs on server
    // Additional validation for extra security
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Unauthorized ability grant attempt blocked!"));
        return;
    }
    
    GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
}
```

## Implementation Examples

Let's examine practical implementation patterns that demonstrate how to effectively use the base class and startup ability system.

### Example 1: Basic Attack Ability

A fundamental melee attack ability that showcases common patterns:

```cpp
// Header: TDAbility_BasicAttack.h
UCLASS()
class RPG_TOPDOWN_API UTDAbility_BasicAttack : public UTDGameplayAbilityBase
{
    GENERATED_BODY()

public:
    UTDAbility_BasicAttack();

    virtual void ActivateAbility(/*...*/) override;

protected:
    /** Attack montage to play */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimMontage> AttackMontage;

    /** Damage to apply on hit */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float BaseDamage = 25.0f;

    /** Range of the attack */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 150.0f;

    /** Gameplay Effect applied for damage */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> DamageGameplayEffect;

    // Task completion callbacks
    UFUNCTION()
    void OnAttackMontageCompleted();

    UFUNCTION()
    void OnAttackMontageCancelled();

    /** Perform the actual attack logic */
    void ExecuteAttack();
};

// Implementation: TDAbility_BasicAttack.cpp
UTDAbility_BasicAttack::UTDAbility_BasicAttack()
{
    // Set ability identification tags
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat.BasicAttack"));
    
    // Prevent simultaneous attacks
    BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat"));
    
    // Set as a combat ability
    CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Movement.Dodge"));
    
    // Configure for immediate activation
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Combat.Attacking"));
    
    // Description for designers
    AbilityDescription = TEXT("Basic melee attack ability. Plays attack animation and applies damage to nearby enemies.");
}

void UTDAbility_BasicAttack::ActivateAbility(/*...*/)
{
    // Call base class first
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Commit the ability (apply costs and cooldowns)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to commit BasicAttack ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Play attack animation
    if (IsValid(AttackMontage))
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = 
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this, TEXT("AttackMontage"), AttackMontage);

        MontageTask->OnCompleted.AddDynamic(this, &UTDAbility_BasicAttack::OnAttackMontageCompleted);
        MontageTask->OnCancelled.AddDynamic(this, &UTDAbility_BasicAttack::OnAttackMontageCancelled);
        MontageTask->ReadyForActivation();

        // Execute attack logic at appropriate animation frame
        // This could be triggered by an animation notify instead
        ExecuteAttack();
    }
    else
    {
        // No animation - execute immediately
        ExecuteAttack();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UTDAbility_BasicAttack::ExecuteAttack()
{
    // Only server should apply damage
    if (!HasAuthority(CurrentActorInfo))
    {
        return;
    }

    ATDCharacterBase* Character = GetTDCharacterFromActorInfo();
    if (!IsValid(Character))
    {
        return;
    }

    // Find targets in range (simplified sphere trace)
    FVector AttackOrigin = Character->GetActorLocation();
    TArray<FHitResult> HitResults;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        AttackOrigin,
        AttackOrigin + Character->GetActorForwardVector() * AttackRange,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(50.0f),
        QueryParams
    );

    // Apply damage to hit targets
    if (bHit && IsValid(DamageGameplayEffect))
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (UAbilitySystemComponent* TargetASC = 
                    UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
                {
                    // Create effect context with hit result information
                    FGameplayEffectContextHandle EffectContext = 
                        GetAbilitySystemComponent()->MakeEffectContext();
                    EffectContext.AddHitResult(Hit);

                    // Apply damage effect
                    FGameplayEffectSpecHandle SpecHandle = 
                        GetAbilitySystemComponent()->MakeOutgoingSpec(
                            DamageGameplayEffect, GetAbilityLevel(), EffectContext);

                    GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(
                        *SpecHandle.Data.Get(), TargetASC);

                    UE_LOG(LogTemp, Log, TEXT("Applied damage to %s"), *HitActor->GetName());
                }
            }
        }
    }
}

void UTDAbility_BasicAttack::OnAttackMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UTDAbility_BasicAttack::OnAttackMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
```

### Example 2: Passive Ability (Always Active)

Some abilities should remain active throughout the character's lifetime:

```cpp
// Header: TDAbility_HealthRegeneration.h
UCLASS()
class RPG_TOPDOWN_API UTDAbility_HealthRegeneration : public UTDGameplayAbilityBase
{
    GENERATED_BODY()

public:
    UTDAbility_HealthRegeneration();

    virtual void ActivateAbility(/*...*/) override;
    virtual void EndAbility(/*...*/) override;

protected:
    /** Health regeneration rate per second */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Regeneration")
    float HealthPerSecond = 5.0f;

    /** Regeneration effect to apply */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> RegenerationEffect;

    /** Handle to the applied regeneration effect */
    FActiveGameplayEffectHandle RegenerationEffectHandle;
};

// Implementation
UTDAbility_HealthRegeneration::UTDAbility_HealthRegeneration()
{
    // Configure as a passive ability
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    
    // This ability should auto-activate and never end
    bServerRespectsRemoteAbilityCancellation = false;
    bRetriggerInstancedAbility = false;
    
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Passive.HealthRegen"));
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.HealthRegenerating"));
    
    AbilityDescription = TEXT("Passive health regeneration ability. Continuously restores health over time.");
}

void UTDAbility_HealthRegeneration::ActivateAbility(/*...*/)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Apply ongoing regeneration effect
    if (HasAuthority(ActorInfo) && IsValid(RegenerationEffect))
    {
        FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
            RegenerationEffect, GetAbilityLevel(), EffectContext);

        RegenerationEffectHandle = GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(
            *SpecHandle.Data.Get());

        UE_LOG(LogTemp, Log, TEXT("Health regeneration started for %s"), 
               ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("Unknown"));
    }

    // This ability doesn't end automatically - it runs indefinitely
}

void UTDAbility_HealthRegeneration::EndAbility(/*...*/)
{
    // Clean up the regeneration effect when ability ends
    if (RegenerationEffectHandle.IsValid() && HasAuthority(ActorInfo))
    {
        GetAbilitySystemComponent()->RemoveActiveGameplayEffect(RegenerationEffectHandle);
        RegenerationEffectHandle.Invalidate();
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

### Conditional Ability Granting

Sometimes abilities should only be granted under specific conditions:

```cpp
void ATDCharacterBase::GrantAbilitiesForLevel(int32 NewLevel)
{
    if (!HasAuthority())
        return;

    // Define level-based ability unlocks
    TMap<int32, TArray<TSubclassOf<UGameplayAbility>>> LevelAbilities = {
        {1, {UTDAbility_BasicAttack::StaticClass()}},
        {3, {UTDAbility_PowerAttack::StaticClass()}},
        {5, {UTDAbility_MagicMissile::StaticClass()}},
        {10, {UTDAbility_Fireball::StaticClass()}}
    };

    if (LevelAbilities.Contains(NewLevel))
    {
        for (const TSubclassOf<UGameplayAbility>& AbilityClass : LevelAbilities[NewLevel])
        {
            FGameplayAbilitySpec AbilitySpec(AbilityClass, NewLevel, INDEX_NONE, this);
            AbilitySystemComponent->GiveAbility(AbilitySpec);

            UE_LOG(LogTemp, Log, TEXT("Granted level %d ability: %s"), 
                   NewLevel, *AbilityClass->GetName());
        }
    }
}
```

### Equipment-Based Ability Granting

Abilities can be granted and removed based on equipped items:

```cpp
void ATDCharacterBase::OnWeaponEquipped(ATDWeapon* NewWeapon)
{
    if (!HasAuthority() || !IsValid(NewWeapon))
        return;

    // Remove old weapon abilities
    if (IsValid(CurrentWeapon))
    {
        RemoveWeaponAbilities(CurrentWeapon);
    }

    // Grant new weapon abilities
    CurrentWeapon = NewWeapon;
    GrantWeaponAbilities(NewWeapon);
}

void ATDCharacterBase::GrantWeaponAbilities(ATDWeapon* Weapon)
{
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : Weapon->GetGrantedAbilities())
    {
        FGameplayAbilitySpec AbilitySpec(AbilityClass, GetCharacterLevel(), INDEX_NONE, Weapon);
        FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
        
        // Track weapon-granted abilities for removal
        WeaponGrantedAbilities.Add(Handle);
    }
}

void ATDCharacterBase::RemoveWeaponAbilities(ATDWeapon* Weapon)
{
    for (const FGameplayAbilitySpecHandle& Handle : WeaponGrantedAbilities)
    {
        AbilitySystemComponent->ClearAbility(Handle);
    }
    WeaponGrantedAbilities.Empty();
}
```

## Advanced Patterns

### Dynamic Ability Loading

For large projects, you might want to load abilities dynamically:

```cpp
void ATDCharacterBase::LoadAndGrantAbilitiesAsync(const TArray<FSoftClassPath>& AbilityPaths)
{
    if (!HasAuthority())
        return;

    // Asynchronously load ability classes
    TArray<FSoftObjectPath> SoftPaths;
    for (const FSoftClassPath& Path : AbilityPaths)
    {
        SoftPaths.Add(Path.ToSoftObjectPath());
    }

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
    StreamableManager.RequestAsyncLoad(SoftPaths, 
        FStreamableDelegate::CreateUObject(this, &ATDCharacterBase::OnAbilitiesLoaded, SoftPaths));
}

void ATDCharacterBase::OnAbilitiesLoaded(TArray<FSoftObjectPath> LoadedPaths)
{
    for (const FSoftObjectPath& Path : LoadedPaths)
    {
        if (UClass* LoadedClass = Cast<UClass>(Path.ResolveObject()))
        {
            if (LoadedClass->IsChildOf<UGameplayAbility>())
            {
                FGameplayAbilitySpec AbilitySpec(LoadedClass, 1, INDEX_NONE, this);
                AbilitySystemComponent->GiveAbility(AbilitySpec);
            }
        }
    }
}
```

### Ability Set Data Assets

Create data assets for organized ability management:

```cpp
// Data Asset for ability collections
UCLASS(BlueprintType)
class RPG_TOPDOWN_API UTDAbilitySet : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** Abilities granted by this set */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
    TArray<FTDAbilitySetEntry> GrantedAbilities;

    /** Gameplay Effects applied by this set */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    TArray<FTDGameplayEffectSetEntry> GrantedEffects;

    /** Attribute sets granted by this set */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TArray<TSubclassOf<UAttributeSet>> GrantedAttributeSets;

    /** Grant all abilities, effects, and attributes from this set */
    void GrantToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject = nullptr);
};

// Ability set entry structure
USTRUCT(BlueprintType)
struct FTDAbilitySetEntry
{
    GENERATED_BODY()

    /** Ability to grant */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> Ability;

    /** Level to grant the ability at */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 AbilityLevel = 1;

    /** Input action to bind this ability to */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UInputAction> InputAction;
};
```

### Conditional Startup Abilities

Grant different abilities based on character configuration:

```cpp
void ATDCharacterBase::GrantConditionalStartupAbilities()
{
    if (!HasAuthority())
        return;

    // Grant class-specific abilities
    if (CharacterClass == ECharacterClass::Warrior)
    {
        GrantAbilitiesFromSet(WarriorAbilitySet);
    }
    else if (CharacterClass == ECharacterClass::Mage)
    {
        GrantAbilitiesFromSet(MageAbilitySet);
    }

    // Grant abilities based on starting equipment
    for (ATDItem* StartingItem : StartingEquipment)
    {
        if (IsValid(StartingItem))
        {
            GrantAbilitiesFromItem(StartingItem);
        }
    }

    // Grant abilities based on character traits
    if (HasTrait(ECharacterTrait::FastLearner))
    {
        GrantAbility(UTDAbility_ExperienceBoost::StaticClass(), 1);
    }
}
```

## Integration with Project Architecture

Understanding how the base ability class and startup granting integrate with the existing TD RPG TopDown architecture ensures seamless implementation.

### Integration with UTDAbilitySystemComponent

The project's custom ASC can provide enhanced ability management:

```cpp
// Enhanced UTDAbilitySystemComponent for base ability support
class UTDAbilitySystemComponent : public UGASCoreAbilitySystemComponent
{
public:
    /** Grant ability and return success status */
    UFUNCTION(BlueprintCallable, Category = "TD|Abilities")
    bool GrantTDAbility(TSubclassOf<UTDGameplayAbilityBase> AbilityClass, 
                        int32 Level = 1, 
                        UObject* SourceObject = nullptr);

    /** Get all abilities of a specific base type */
    UFUNCTION(BlueprintCallable, Category = "TD|Abilities")
    TArray<FGameplayAbilitySpec*> GetAbilitiesByBaseClass(
        TSubclassOf<UTDGameplayAbilityBase> BaseClass);

    /** Check if character has any abilities of specific type */
    UFUNCTION(BlueprintCallable, Category = "TD|Abilities")
    bool HasAbilityOfClass(TSubclassOf<UTDGameplayAbilityBase> AbilityClass) const;

protected:
    /** Override to add TD-specific ability granting logic */
    virtual FGameplayAbilitySpecHandle GiveAbility(const FGameplayAbilitySpec& Spec) override;
};
```

### Integration with ATDCharacterBase

The character base class works seamlessly with the ability system:

```cpp
// Enhanced character initialization
void ATDCharacterBase::InitializeAbilityActorInfo()
{
    // ... existing ASC initialization ...

    // Bind enhanced delegates for ability tracking
    if (UTDAbilitySystemComponent* TDASC = Cast<UTDAbilitySystemComponent>(AbilitySystemComponent))
    {
        TDASC->BindASCDelegates();
        
        // Listen for ability activations for UI updates
        TDASC->AbilityActivatedDelegate.AddDynamic(this, &ATDCharacterBase::OnAbilityActivated);
        TDASC->AbilityEndedDelegate.AddDynamic(this, &ATDCharacterBase::OnAbilityEnded);
    }

    // Grant startup abilities after ASC is fully configured
    if (bAutoGrantStartupAbilities && HasAuthority())
    {
        GrantStartupAbilities();
        
        // Grant passive abilities that should auto-activate
        ActivatePassiveAbilities();
    }
}

UFUNCTION()
void ATDCharacterBase::OnAbilityActivated(UGameplayAbility* ActivatedAbility)
{
    // React to ability activations (UI updates, sound effects, etc.)
    if (UTDGameplayAbilityBase* TDAbility = Cast<UTDGameplayAbilityBase>(ActivatedAbility))
    {
        // Project-specific handling
        OnTDAbilityActivated(TDAbility);
    }
}

void ATDCharacterBase::ActivatePassiveAbilities()
{
    if (!HasAuthority())
        return;

    // Auto-activate abilities tagged as passive
    FGameplayTagContainer PassiveTag(FGameplayTag::RequestGameplayTag("Ability.Type.Passive"));
    AbilitySystemComponent->TryActivateAbilitiesByTag(PassiveTag);
}
```

### Integration with ATDPlayerState

For player characters, the PlayerState manages persistent ability state:

```cpp
// ATDPlayerState ability management
void ATDPlayerState::GrantPlayerStartupAbilities()
{
    if (!HasAuthority())
        return;

    // Grant abilities that persist across possession changes
    TArray<TSubclassOf<UGameplayAbility>> PersistentAbilities = {
        UTDAbility_Inventory::StaticClass(),
        UTDAbility_CharacterSheet::StaticClass(),
        UTDAbility_MenuAccess::StaticClass()
    };

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : PersistentAbilities)
    {
        if (IsValid(AbilityClass))
        {
            FGameplayAbilitySpec AbilitySpec(AbilityClass, GetPlayerLevel(), INDEX_NONE, this);
            AbilitySystemComponent->GiveAbility(AbilitySpec);
        }
    }
}

// Handle player level changes
void ATDPlayerState::OnPlayerLevelChanged(int32 NewLevel)
{
    // Update ability levels to match player level
    if (HasAuthority() && IsValid(AbilitySystemComponent))
    {
        TArray<FGameplayAbilitySpec*> AllSpecs;
        AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
            FGameplayTagContainer(), AllSpecs);

        for (FGameplayAbilitySpec* Spec : AllSpecs)
        {
            if (Spec && Spec->Ability->IsA<UTDGameplayAbilityBase>())
            {
                // Update spec level
                Spec->Level = NewLevel;
                
                // Mark for replication
                AbilitySystemComponent->MarkAbilitySpecDirty(*Spec);
            }
        }
    }
}
```

### Blueprint Integration Patterns

Designers can leverage the base class in Blueprints:

#### Blueprint Setup
1. **Create Blueprint**: New Blueprint Class → Parent Class: "TD Gameplay Ability Base"
2. **Configure Properties**: Set ability tags, costs, cooldowns in details panel
3. **Override Events**: Implement `OnAbilityStart` and `OnAbilityEnd` events
4. **Use Helper Functions**: Call `GetTDCharacterFromActorInfo` and similar utilities

#### Blueprint Event Graph Example
```
Event OnAbilityStart
├── Get TD Character From Actor Info
├── Branch (Is Valid)
│   ├── True → Play Sound at Location
│   └── False → Print String ("No valid character found")
├── Wait Delay (2.0 seconds)
└── End Ability
```

## Common Pitfalls

### Pitfall 1: Incorrect Granting Timing

**Problem**: Granting abilities before the ASC is properly initialized.

```cpp
// ❌ WRONG: Too early granting
void ATDCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    // ASC might not be initialized yet!
    GrantStartupAbilities();
}

// ✅ CORRECT: Grant after proper initialization
void ATDCharacterBase::InitializeAbilityActorInfo()
{
    // ... ASC initialization ...
    
    // NOW it's safe to grant abilities
    if (bAutoGrantStartupAbilities && HasAuthority())
    {
        GrantStartupAbilities();
    }
}
```

### Pitfall 2: Missing Authority Checks

**Problem**: Attempting to grant abilities on clients or non-authority actors.

```cpp
// ❌ WRONG: No authority validation
void ATDCharacterBase::AddNewAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
    // This could run on client - security risk!
    AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
}

// ✅ CORRECT: Proper authority validation
void ATDCharacterBase::AddNewAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Attempted to grant ability on non-authority!"));
        return;
    }
    
    if (!IsValid(AbilitySystemComponent))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot grant ability: Invalid ASC"));
        return;
    }
    
    AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
}
```

### Pitfall 3: Memory Management Issues

**Problem**: Holding strong references to abilities or failing to clean up properly.

```cpp
// ❌ WRONG: Holding unnecessary strong references
UPROPERTY()
TArray<UGameplayAbility*> CachedAbilityInstances; // Memory leak risk!

// ✅ CORRECT: Use handles or weak references
TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

// Or use weak references for instance tracking
TArray<TWeakObjectPtr<UGameplayAbility>> WeakAbilityReferences;
```

### Pitfall 4: Blueprint Compilation Dependencies

**Problem**: Creating circular dependencies between ability blueprints and character blueprints.

**Solution**: Use forward declarations and soft references:

```cpp
// Use soft class references for abilities to avoid hard dependencies
UPROPERTY(EditDefaultsOnly, Category = "Abilities")
TArray<TSoftClassPtr<UGameplayAbility>> StartupAbilitiesSoft;

// Load them when needed
void LoadStartupAbilities()
{
    StartupAbilities.Empty();
    for (const TSoftClassPtr<UGameplayAbility>& SoftAbility : StartupAbilitiesSoft)
    {
        if (UClass* AbilityClass = SoftAbility.LoadSynchronous())
        {
            StartupAbilities.Add(AbilityClass);
        }
    }
}
```

### Pitfall 5: Forgetting Blueprint Compatibility

**Problem**: Base class implementation that doesn't work well with Blueprint inheritance.

```cpp
// ❌ PROBLEMATIC: Hard to use from Blueprints
virtual void ActivateAbility(/*complex parameters*/) override final
{
    // Final keyword prevents Blueprint override
    // Complex logic that Blueprints can't extend
}

// ✅ BLUEPRINT-FRIENDLY: Extensible design
virtual void ActivateAbility(/*...*/) override
{
    // Call Blueprint-implementable event
    OnAbilityStart();
    
    // Core logic that Blueprints can extend
    Super::ActivateAbility(/*...*/);
}

// Blueprint event for extensibility
UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
void OnAbilityStart();
```

## Testing and Validation

### Unit Testing Base Class Functionality

```cpp
// Test framework integration
class FTDAbilityBaseTest
{
public:
    static bool TestBasicActivation()
    {
        // Create test environment
        UWorld* TestWorld = CreateTestWorld();
        ATDCharacterBase* TestCharacter = TestWorld->SpawnActor<ATDCharacterBase>();
        
        // Initialize ASC
        TestCharacter->InitializeAbilityActorInfo();
        
        // Grant test ability
        TSubclassOf<UTDGameplayAbilityBase> TestAbilityClass = UTDTestAbility::StaticClass();
        TestCharacter->GrantAbility(TestAbilityClass, 1);
        
        // Verify granting
        bool bHasAbility = TestCharacter->GetAbilitySystemComponent()
            ->FindAbilitySpecFromClass(TestAbilityClass) != nullptr;
            
        return bHasAbility;
    }

    static bool TestStartupAbilityGranting()
    {
        // Test automatic startup ability granting
        // ... implementation ...
    }
};
```

### Integration Testing

Test the complete flow from character creation to ability usage:

```cpp
void TestCompleteAbilityFlow()
{
    // 1. Create character
    ATDPlayerCharacter* Player = SpawnTestPlayer();
    
    // 2. Initialize (should grant startup abilities)
    Player->InitializeAbilityActorInfo();
    
    // 3. Verify startup abilities were granted
    VerifyStartupAbilities(Player);
    
    // 4. Test ability activation
    TestAbilityActivation(Player);
    
    // 5. Test ability cleanup
    TestAbilityCleanup(Player);
}
```

### Performance Testing

Monitor the performance impact of your base class and startup system:

```cpp
void ATDCharacterBase::GrantStartupAbilities()
{
    SCOPE_CYCLE_COUNTER(STAT_StartupAbilityGranting);
    
    double StartTime = FPlatformTime::Seconds();
    
    // ... granting logic ...
    
    double ElapsedTime = FPlatformTime::Seconds() - StartTime;
    UE_LOG(LogTemp, Log, TEXT("Startup ability granting took %f ms"), ElapsedTime * 1000.0);
}
```

## Best Practices Checklist

### Base Class Design
- [ ] **Minimal Initial Implementation**: Start with essential features only
- [ ] **Blueprint Compatibility**: Ensure Blueprint inheritance works smoothly
- [ ] **Consistent Defaults**: Set sensible default policies for all abilities
- [ ] **Extension Points**: Provide virtual functions and events for customization
- [ ] **Project Integration**: Include project-specific utility functions

### Startup Ability System
- [ ] **Editable Arrays**: Expose ability arrays in character details panel
- [ ] **Authority Validation**: All granting happens server-side only
- [ ] **Proper Timing**: Grant abilities after ASC initialization is complete
- [ ] **Error Handling**: Gracefully handle invalid ability classes
- [ ] **Logging**: Provide clear logs for debugging granting issues

### Network Security
- [ ] **Server Authority**: All ability granting is server-authoritative
- [ ] **Input Validation**: Validate ability classes before granting
- [ ] **Audit Logging**: Log ability grants for security monitoring
- [ ] **Blueprint Protection**: Use BlueprintAuthorityOnly where appropriate

### Performance Optimization
- [ ] **Batch Operations**: Grant multiple abilities efficiently
- [ ] **Lazy Loading**: Load abilities only when needed if using large sets
- [ ] **Memory Management**: Avoid unnecessary strong references
- [ ] **Profile Regularly**: Monitor performance impact of ability systems

### Maintainability
- [ ] **Clear Documentation**: Document base class purpose and usage
- [ ] **Consistent Naming**: Follow project naming conventions
- [ ] **Version Control**: Track changes to base class carefully
- [ ] **Team Communication**: Ensure team understands base class changes

### Testing Coverage
- [ ] **Unit Tests**: Test base class functionality in isolation
- [ ] **Integration Tests**: Test complete character initialization flow
- [ ] **Multiplayer Tests**: Verify network behavior with multiple clients
- [ ] **Performance Tests**: Ensure no significant performance regressions
- [ ] **Edge Case Testing**: Test with invalid data, network interruptions, etc.

This comprehensive guide provides the foundation for implementing robust, maintainable gameplay ability systems in your Unreal Engine projects. The patterns described here scale from simple indie games to complex AAA productions, providing a solid architectural foundation for your gameplay programming needs.