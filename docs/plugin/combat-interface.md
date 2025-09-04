# GASCore Combat Interface

Last updated: 2024-12-19

## Overview

The `IGASCoreCombatInterface` provides a lightweight, decoupled interface for exposing combat-related queries throughout the gameplay system. It serves as a bridge between different systems that need combat information without requiring tight coupling to specific character or component implementations.

## Interface Definition

```cpp
/**
 * ICombatInterface
 *
 * Lightweight gameplay interface exposing combat-related queries to decouple systems.
 * Current responsibility:
 * - Provide an Actor's level as a non-Attribute integer for calculations (e.g., MMCs).
 *
 * Design notes:
 * - Using a plain virtual here (not UFUNCTION). That keeps it lightweight and allows
 *   direct C++ calls via native pointers. If you need Blueprint implementability
 *   or network RPC hooks, make it a UFUNCTION(BlueprintNativeEvent).
 */
class GASCORE_API IGASCoreCombatInterface
{
    GENERATED_BODY()

public:
    // Return the actor's level for gameplay calculations (e.g., MaxHealth MMC).
    // Implementation can fetch from PlayerState (players) or the Character itself (AI).
    // Contract: Must be fast and safe to call during effect evaluation (no blocking).
    virtual int32 GetActorLevel();
};
```

## Core Design Principles

### Lightweight Implementation
- **No UFUNCTION Overhead:** Uses plain virtual functions for maximum performance
- **Minimal Dependencies:** No heavy includes or complex dependencies
- **Direct C++ Calls:** Allows efficient native pointer access without Blueprint overhead

### Decoupling Benefits
- **System Independence:** Combat calculations don't need to know character implementation details
- **Flexible Ownership:** Level can come from PlayerState, Character, or any other source
- **Easy Testing:** Simple interface makes mocking and unit testing straightforward

### Performance Contract
- **Fast Execution:** Must complete quickly during effect evaluation
- **No Blocking:** Cannot perform async operations or expensive calculations
- **Safe Calling:** Must be safe to call from any thread during effect processing

## Implementation Patterns

### Player Character Implementation

```cpp
class RPG_TOPDOWN_API ATDPlayerCharacter : public ATDCharacterBase, public IGASCoreCombatInterface
{
public:
    // IGASCoreCombatInterface
    virtual int32 GetActorLevel() override
    {
        // Fetch from PlayerState for persistence across respawns
        if (ATDPlayerState* TDPS = GetPlayerState<ATDPlayerState>())
        {
            return TDPS->GetCharacterLevel();
        }
        
        // Fallback for edge cases (should rarely happen)
        return 1;
    }
};
```

**Benefits:**
- **Persistent Level:** Survives character respawning
- **Centralized Data:** Level management handled by PlayerState
- **Network Replication:** PlayerState handles replication automatically

### AI Character Implementation

```cpp
class RPG_TOPDOWN_API ATDEnemyCharacter : public ATDCharacterBase, public IGASCoreCombatInterface
{
protected:
    /** AI-specific level for difficulty scaling */
    UPROPERTY(EditAnywhere, Category = "Character Class Defaults")
    int32 EnemyCharacterLevel = 1;

public:
    // IGASCoreCombatInterface
    virtual int32 GetActorLevel() override
    {
        // AI enemies store level directly on character
        return EnemyCharacterLevel;
    }
};
```

**Benefits:**
- **Simple Storage:** Level stored directly where it's used
- **Designer Control:** Easily configurable per enemy type in Blueprint
- **Immediate Availability:** No dependency on external systems

### Generic Implementation for Any Actor

```cpp
class MYGAME_API AGenericCombatActor : public AActor, public IGASCoreCombatInterface
{
protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    int32 ActorLevel = 1;

public:
    // IGASCoreCombatInterface
    virtual int32 GetActorLevel() override
    {
        return ActorLevel;
    }
};
```

## Usage in Gameplay Systems

### Magnitude Calculation Classes (MMCs)

The primary use case for combat interface is in MMC calculations where level-based scaling is needed:

```cpp
class GASCORE_API UMaxHealthMMC : public UGameplayModMagnitudeCalculation
{
public:
    UMaxHealthMMC();

    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};

float UMaxHealthMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    // Get attribute values
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;
    
    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
    Vigor = FMath::Max<float>(Vigor, 0.f);
    
    // Get actor level via combat interface
    int32 CharacterLevel = 1;
    if (AActor* SourceActor = Spec.GetEffectContext().GetSourceObject<AActor>())
    {
        if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(SourceActor))
        {
            CharacterLevel = CombatInterface->GetActorLevel();
        }
    }
    
    // Calculate level-scaled health
    // Formula: (Vigor * 2.5) + (Level * 10) + 85
    float MaxHealth = (Vigor * 2.5f) + (CharacterLevel * 10.f) + 85.f;
    
    return MaxHealth;
}
```

### Damage Calculation Systems

```cpp
float CalculateDamageWithLevel(AActor* Attacker, AActor* Target, float BaseDamage)
{
    int32 AttackerLevel = 1;
    int32 TargetLevel = 1;
    
    // Get attacker level
    if (IGASCoreCombatInterface* AttackerCombat = Cast<IGASCoreCombatInterface>(Attacker))
    {
        AttackerLevel = AttackerCombat->GetActorLevel();
    }
    
    // Get target level  
    if (IGASCoreCombatInterface* TargetCombat = Cast<IGASCoreCombatInterface>(Target))
    {
        TargetLevel = TargetCombat->GetActorLevel();
    }
    
    // Apply level difference scaling
    float LevelDifference = AttackerLevel - TargetLevel;
    float LevelMultiplier = 1.0f + (LevelDifference * 0.1f); // 10% per level difference
    
    return BaseDamage * FMath::Max(0.1f, LevelMultiplier); // Minimum 10% damage
}
```

### Experience/Progression Systems

```cpp
int32 CalculateExperienceReward(AActor* KilledActor, AActor* KillerActor)
{
    int32 BaseXP = 100;
    
    // Get level information
    int32 KilledLevel = 1;
    int32 KillerLevel = 1;
    
    if (IGASCoreCombatInterface* KilledCombat = Cast<IGASCoreCombatInterface>(KilledActor))
    {
        KilledLevel = KilledCombat->GetActorLevel();
    }
    
    if (IGASCoreCombatInterface* KillerCombat = Cast<IGASCoreCombatInterface>(KillerActor))
    {
        KillerLevel = KillerCombat->GetActorLevel();
    }
    
    // Scale XP based on level difference
    int32 LevelDifference = KilledLevel - KillerLevel;
    float XPMultiplier = FMath::Clamp(1.0f + (LevelDifference * 0.2f), 0.1f, 3.0f);
    
    return FMath::RoundToInt(BaseXP * KilledLevel * XPMultiplier);
}
```

## Advanced Usage Patterns

### Conditional Interface Checking

```cpp
bool IsValidCombatTarget(AActor* PotentialTarget)
{
    if (!PotentialTarget)
        return false;
    
    // Check if actor implements combat interface
    if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(PotentialTarget))
    {
        // Additional validation based on level
        int32 TargetLevel = CombatInterface->GetActorLevel();
        
        // Example: Don't allow targeting actors more than 10 levels higher
        if (TargetLevel > GetPlayerLevel() + 10)
        {
            return false;
        }
        
        return true;
    }
    
    return false; // No combat interface = not a valid combat target
}
```

### Blueprint Integration (Optional)

If Blueprint access is needed, you can add UFUNCTION versions alongside the native interface:

```cpp
class GASCORE_API IGASCoreCombatInterface
{
    GENERATED_BODY()

public:
    // Native C++ interface (performance)
    virtual int32 GetActorLevel() { return 1; }
    
    // Blueprint accessible version (convenience)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Combat")
    int32 GetActorLevel_BP();
};
```

Usage in Blueprint:
```cpp
// Check if actor implements interface and get level
if (UKismetSystemLibrary::DoesImplementInterface(Actor, UGASCoreCombatInterface::StaticClass()))
{
    int32 Level = IGASCoreCombatInterface::Execute_GetActorLevel_BP(Actor);
    // Use level...
}
```

## Interface Extension Patterns

### Adding New Combat Queries

```cpp
class GASCORE_API IGASCoreCombatInterface
{
    GENERATED_BODY()

public:
    // Core level query
    virtual int32 GetActorLevel() { return 1; }
    
    // Extended combat information
    virtual float GetArmorValue() { return 0.f; }
    virtual float GetMagicResistance() { return 0.f; }
    virtual bool CanBeTargeted() { return true; }
    virtual FVector GetCombatLocation() { return FVector::ZeroVector; }
    
    // Status queries
    virtual bool IsInCombat() { return false; }
    virtual bool IsHostileTo(AActor* OtherActor) { return false; }
    virtual ECombatTeam GetCombatTeam() { return ECombatTeam::Neutral; }
};
```

### Team-Based Implementation

```cpp
class RPG_TOPDOWN_API ATDCharacterBase : public ACharacter, public IGASCoreCombatInterface
{
protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    ECombatTeam CombatTeam = ECombatTeam::Player;

public:
    // IGASCoreCombatInterface extensions
    virtual bool IsHostileTo(AActor* OtherActor) override
    {
        if (IGASCoreCombatInterface* OtherCombat = Cast<IGASCoreCombatInterface>(OtherActor))
        {
            ECombatTeam OtherTeam = OtherCombat->GetCombatTeam();
            return CombatTeam != OtherTeam && CombatTeam != ECombatTeam::Neutral && OtherTeam != ECombatTeam::Neutral;
        }
        return false;
    }
    
    virtual ECombatTeam GetCombatTeam() override
    {
        return CombatTeam;
    }
};
```

## Performance Optimization

### Interface Caching

```cpp
class GASCORE_API UCombatManager : public USubsystem
{
    GENERATED_BODY()

public:
    /** Cache frequently accessed combat interfaces */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CacheCombatInterface(AActor* Actor);
    
    /** Get cached level (faster than repeated interface calls) */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 GetCachedActorLevel(AActor* Actor);

private:
    /** Cached level data */
    UPROPERTY()
    TMap<AActor*, int32> CachedLevels;
    
    /** Cache refresh timer */
    FTimerHandle CacheRefreshTimer;
    
    /** Refresh cached data periodically */
    void RefreshCacheData();
};
```

### Bulk Operations

```cpp
void ApplyAreaEffect(const TArray<AActor*>& Targets, float BaseDamage, AActor* Source)
{
    // Pre-calculate source level once
    int32 SourceLevel = 1;
    if (IGASCoreCombatInterface* SourceCombat = Cast<IGASCoreCombatInterface>(Source))
    {
        SourceLevel = SourceCombat->GetActorLevel();
    }
    
    // Process all targets efficiently
    for (AActor* Target : Targets)
    {
        if (IGASCoreCombatInterface* TargetCombat = Cast<IGASCoreCombatInterface>(Target))
        {
            int32 TargetLevel = TargetCombat->GetActorLevel();
            float ScaledDamage = CalculateLevelScaledDamage(BaseDamage, SourceLevel, TargetLevel);
            ApplyDamage(Target, ScaledDamage);
        }
    }
}
```

## Testing and Debugging

### Unit Testing Interface Implementation

```cpp
UCLASS()
class UTestCombatActor : public UObject, public IGASCoreCombatInterface
{
    GENERATED_BODY()

public:
    int32 TestLevel = 5;
    
    // IGASCoreCombatInterface
    virtual int32 GetActorLevel() override { return TestLevel; }
};

// Test case
void TestCombatInterface()
{
    UTestCombatActor* TestActor = NewObject<UTestCombatActor>();
    TestActor->TestLevel = 10;
    
    IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(TestActor);
    check(CombatInterface != nullptr);
    check(CombatInterface->GetActorLevel() == 10);
}
```

### Debug Utilities

```cpp
// Console command to display combat info for all actors
UFUNCTION(Exec, Category = "Debug")
void DebugCombatInterfaces()
{
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(Actor))
        {
            int32 Level = CombatInterface->GetActorLevel();
            UE_LOG(LogTemp, Warning, TEXT("Actor: %s, Level: %d"), *Actor->GetName(), Level);
        }
    }
}
```

## Migration and Backwards Compatibility

### Adding Interface to Existing Characters

```cpp
// Before: Character without interface
class ATDOldCharacter : public ACharacter
{
    // No combat interface...
};

// After: Add interface implementation
class ATDOldCharacter : public ACharacter, public IGASCoreCombatInterface
{
public:
    // Add interface implementation
    virtual int32 GetActorLevel() override
    {
        // Default implementation for existing characters
        return CharacterLevel.IsValid() ? CharacterLevel : 1;
    }

private:
    // Add level storage if not present
    UPROPERTY(EditAnywhere, Category = "Combat")
    int32 CharacterLevel = 1;
};
```

### Gradual Interface Adoption

```cpp
// Utility function for safe interface access during migration
int32 GetActorLevelSafe(AActor* Actor)
{
    if (!Actor)
        return 1;
    
    // Try combat interface first
    if (IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(Actor))
    {
        return CombatInterface->GetActorLevel();
    }
    
    // Fallback for legacy characters
    if (ATDCharacterBase* Character = Cast<ATDCharacterBase>(Actor))
    {
        // Use legacy method if available
        return Character->GetLegacyLevel();
    }
    
    // Final fallback
    return 1;
}
```

## See Also

- [Character Base Implementation](character-base.md)
- [MMC Implementation Guide](../gas/attributes/custom-calculations.md)
- [Gameplay Effects](../gas/gameplay-effects.md)
- [Actor Level Design Document](../gdd/actor-level-progression.md)
- [Performance Optimization](performance.md)