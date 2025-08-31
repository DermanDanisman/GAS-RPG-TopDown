# Base Ability and Startup Grant

Last updated: 2025-01-06

## Rationale for Base Ability Class

Creating a shared base ability class (`UTDGameplayAbilityBase`) provides consistent behavior and reduces boilerplate across all project abilities.

### Benefits of UTDGameplayAbilityBase

**Standardized Configuration:**
- Common activation/blocking tag patterns
- Consistent input ID management
- Shared cost/cooldown validation
- Project-specific debugging utilities

**Reduced Duplication:**
- Input handling logic centralized
- Common ability patterns (target acquisition, validation)
- Shared helper functions for project-specific needs

**Maintainability:**
- Single point for ability system changes
- Consistent behavior across all abilities
- Easier debugging with shared logging

### Example Base Class Structure
```cpp
// UTDGameplayAbilityBase.h
UCLASS(Abstract, BlueprintType, Blueprintable)
class RPG_TOPDOWN_API UTDGameplayAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UTDGameplayAbilityBase();

protected:
    // ================================================================================================================
    // ACTIVATION TAGS
    // ================================================================================================================
    
    /** Tags required for this ability to activate */
    UPROPERTY(EditDefaultsOnly, Category = "Activation")
    FGameplayTagContainer ActivationRequiredTags;
    
    /** Tags that block this ability from activating */
    UPROPERTY(EditDefaultsOnly, Category = "Activation")
    FGameplayTagContainer ActivationBlockedTags;

    // ================================================================================================================
    // INPUT
    // ================================================================================================================
    
    /** Input ID for binding this ability to input actions */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    int32 AbilityInputID = -1;
    
    // ================================================================================================================
    // VALIDATION HELPERS
    // ================================================================================================================
    
    /** Check if required gameplay tags are present and blocking tags are absent */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TD Ability")
    bool CheckActivationTags() const;
    
    /** Get the character that owns this ability */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TD Ability")
    class ATDCharacterBase* GetTDCharacterFromActorInfo() const;
    
    /** Get the ability system component from actor info */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TD Ability")
    class UAbilitySystemComponent* GetTDAbilitySystemComponentFromActorInfo() const;
};
```

## Character StartupAbilities Array

### Server-Authoritative Granting Timing

Abilities should be granted **after** ASC initialization to ensure proper replication and avoid race conditions.

### Optimal Timing Points

**For Player Characters:**
```cpp
// ATDPlayerCharacter.cpp
void ATDPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    // Server-only: Initialize ASC and grant abilities
    if (ATDPlayerState* TDPS = GetPlayerState<ATDPlayerState>())
    {
        InitializeAbilityActorInfo();
        GrantStartupAbilities(); // Call after ASC initialization
    }
}

void ATDPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    // Client: Initialize ASC (abilities are granted by server and replicated)
    InitializeAbilityActorInfo();
}
```

**For AI Characters:**
```cpp
// ATDEnemyCharacter.cpp
void ATDEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        InitializeAbilityActorInfo();
        GrantStartupAbilities(); // Server-only for AI
    }
}
```

### StartupAbilities Configuration

```cpp
// In character class header
UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
TArray<TSubclassOf<UTDGameplayAbilityBase>> StartupAbilities;

// Optional: Include activation input mappings
UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
TArray<int32> StartupAbilityInputIDs;
```

## ASC API Pattern for Granting Abilities

### Grant with Duplicate Guard

Prevent duplicate ability grants by checking existing specs:

```cpp
void ATDCharacterBase::GrantStartupAbilities()
{
    if (!HasAuthority() || !AbilitySystemComponent || StartupAbilities.IsEmpty())
    {
        return;
    }

    for (int32 i = 0; i < StartupAbilities.Num(); ++i)
    {
        TSubclassOf<UTDGameplayAbilityBase> AbilityClass = StartupAbilities[i];
        if (!AbilityClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("Null ability class at index %d"), i);
            continue;
        }
        
        // Check for duplicates to avoid multiple grants
        if (AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
        {
            UE_LOG(LogTemp, Log, TEXT("Ability %s already granted, skipping"), *AbilityClass->GetName());
            continue;
        }
        
        // Determine input ID
        int32 InputID = -1;
        if (StartupAbilityInputIDs.IsValidIndex(i))
        {
            InputID = StartupAbilityInputIDs[i];
        }
        
        // Grant the ability
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, InputID, this);
        AbilitySystemComponent->GiveAbility(AbilitySpec);
        
        UE_LOG(LogTemp, Log, TEXT("Granted ability: %s with InputID: %d"), 
            *AbilityClass->GetName(), InputID);
    }
}
```

### GiveAbilityAndActivateOnce for Testing

Useful for testing abilities without input setup:

```cpp
void ATDCharacterBase::TestActivateAbility(TSubclassOf<UTDGameplayAbilityBase> AbilityClass)
{
    if (!AbilitySystemComponent || !AbilityClass)
    {
        return;
    }
    
    // Grant and immediately activate (useful for console commands)
    FGameplayAbilitySpec TestSpec(AbilityClass, 1, -1, this);
    AbilitySystemComponent->GiveAbilityAndActivateOnce(TestSpec);
}
```

## Code Sketches

### Character Forwarder Pattern

```cpp
// ATDCharacterBase.h - Base character interface
class RPG_TOPDOWN_API ATDCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    // ... existing code ...

protected:
    /** Abilities granted to this character on initialization */
    UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
    TArray<TSubclassOf<UTDGameplayAbilityBase>> StartupAbilities;
    
    /** Input IDs corresponding to StartupAbilities array */
    UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
    TArray<int32> StartupAbilityInputIDs;

    /** Initialize ASC ActorInfo and grant startup abilities (call after ASC is ready) */
    virtual void InitializeAbilityActorInfo();
    
    /** Grant all abilities in StartupAbilities array (server-only) */
    UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
    virtual void GrantStartupAbilities();
    
public:
    /** Testing utility: grant and activate ability immediately */
    UFUNCTION(BlueprintCallable, Category = "GAS|Debug", CallInEditor = true)
    void TestActivateAbility(TSubclassOf<UTDGameplayAbilityBase> AbilityClass);
};
```

### ASC Grant Loop Implementation

```cpp
// ATDCharacterBase.cpp
void ATDCharacterBase::GrantStartupAbilities()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!HasAuthority() || !ASC || StartupAbilities.IsEmpty())
    {
        return; // Server-only operation
    }

    for (int32 i = 0; i < StartupAbilities.Num(); ++i)
    {
        TSubclassOf<UTDGameplayAbilityBase> AbilityClass = StartupAbilities[i];
        if (!AbilityClass)
        {
            continue; // Skip null entries
        }
        
        // Prevent duplicate grants
        if (ASC->FindAbilitySpecFromClass(AbilityClass))
        {
            continue; // Already granted
        }
        
        // Resolve input ID
        int32 InputID = StartupAbilityInputIDs.IsValidIndex(i) ? StartupAbilityInputIDs[i] : -1;
        
        // Create and grant ability spec
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, InputID, this);
        FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(AbilitySpec);
        
        // Optional: Store handle for later reference
        if (SpecHandle.IsValid())
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Granted ability: %s"), *AbilityClass->GetName());
        }
    }
}
```

## Troubleshooting

### Issue: Abilities Not Granted
**Symptoms:** StartupAbilities array populated but abilities don't activate
**Causes & Solutions:**
1. **ASC not initialized:** Ensure `InitAbilityActorInfo()` called before granting
2. **Client trying to grant:** Add `HasAuthority()` check to granting function
3. **Null ability classes:** Validate `AbilityClass != nullptr` before granting
4. **Missing array setup:** Verify StartupAbilities contains valid ability class references

### Issue: Duplicate Abilities
**Symptoms:** Same ability appears multiple times in ASC specs
**Causes & Solutions:**
1. **Multiple grant calls:** Use `FindAbilitySpecFromClass()` to check before granting
2. **Both server and client granting:** Ensure only server grants (HasAuthority check)
3. **Multiple initialization paths:** Consolidate granting to single code path

### Issue: Input Not Working
**Symptoms:** Abilities granted but don't activate on input
**Causes & Solutions:**
1. **Missing input binding:** Check Enhanced Input setup links InputID to ability
2. **Wrong InputID:** Verify StartupAbilityInputIDs array matches input action mappings
3. **Activation tag conflicts:** Check ActivationRequiredTags/ActivationBlockedTags
4. **ASC not bound to input:** Ensure character's ASC is connected to input system

### Issue: Network Desync
**Symptoms:** Abilities work differently on server vs client
**Causes & Solutions:**
1. **Client granting abilities:** Remove client-side granting code
2. **Missing replication:** Ensure ASC replication mode is set correctly
3. **Prediction issues:** Verify ability's NetExecutionPolicy matches intended behavior
4. **Authority checks:** Add proper HasAuthority() checks in ability logic

### Issue: Abilities Cancel Immediately
**Symptoms:** Abilities start but end instantly without executing
**Causes & Solutions:**
1. **Failed cost/cooldown:** Check `CommitAbility()` return value
2. **Missing resources:** Verify character has required attributes (mana, stamina)
3. **Tag blocking:** Check for unexpected tags in ActivationBlockedTags
4. **Authority mismatch:** Ensure proper server/client execution policy

## Testing Workflow

### 1. Basic Grant Testing
```cpp
// Console command for testing ability grants
void ATDPlayerController::ServerTestGrantAbility_Implementation(TSubclassOf<UTDGameplayAbilityBase> AbilityClass)
{
    if (ATDCharacterBase* Character = Cast<ATDCharacterBase>(GetPawn()))
    {
        Character->TestActivateAbility(AbilityClass);
    }
}
```

### 2. Network Testing
- Test in PIE with multiple clients
- Verify abilities only grant on server
- Check client receives ability specs via replication
- Confirm input activation works on owning client

### 3. Input Integration Testing
- Bind abilities to Enhanced Input actions
- Test activation via keyboard/gamepad input
- Verify input conflicts are resolved appropriately

## See Also

- [Gameplay Abilities Overview](overview.md)
- [Gameplay Effects](../gameplay-effects.md)
- [Replication & Multiplayer](../../architecture/replication-and-multiplayer.md)
- [Debugging and Tools](../../debugging-and-tools.md)