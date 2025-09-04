# TDCharacterBase - Foundation Character Class

Last updated: 2024-12-19

## Overview

`ATDCharacterBase` serves as the abstract foundation for all characters in the RPG TopDown project. It implements essential Gameplay Ability System (GAS) integration, combat interfaces, and provides a structured initialization pattern for both player and AI characters.

## Class Hierarchy and Interfaces

```cpp
UCLASS(Abstract)
class RPG_TOPDOWN_API ATDCharacterBase : public ACharacter, 
                                         public IAbilitySystemInterface, 
                                         public IGASCoreCombatInterface
{
    GENERATED_BODY()
    // Implementation details...
};
```

### Interface Implementations

| Interface | Purpose | Key Methods |
|-----------|---------|-------------|
| `IAbilitySystemInterface` | GAS integration requirement | `GetAbilitySystemComponent()` |
| `IGASCoreCombatInterface` | Combat system integration | `GetActorLevel()` |

## Core Components

### GAS Components

```cpp
/** Ability System Component for this actor.
 * For players, often owned by the PlayerState; for AI, by the Character.
 */
UPROPERTY()
TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

/** AttributeSet for this actor, containing all modifiable gameplay stats. */
UPROPERTY()
TObjectPtr<UAttributeSet> AttributeSet;
```

### Initialization Components

```cpp
/** Component responsible for applying default attribute GameplayEffects. */
UPROPERTY(VisibleAnywhere, Category="GAS|Attributes")
TObjectPtr<UTDDefaultAttributeInitComponent> DefaultAttributeInitComponent;

/** Handles granting abilities and startup logic. */
UPROPERTY(VisibleAnywhere, Category="GAS|Abilities")
TObjectPtr<UTDAbilityInitComponent> AbilityInitComponent;
```

### Visual Components

```cpp
/** Skeletal mesh for the character's weapon. Exposed to the editor for assignment and setup. */
UPROPERTY(EditAnywhere, Category="Combat")
TObjectPtr<USkeletalMeshComponent> WeaponMesh;
```

**Weapon Attachment:**
- Attached to character mesh at `"WeaponHandSocket"` socket
- Collision disabled by default (enable in subclasses for weapon traces)
- Customizable per character type through Blueprint inheritance

## Initialization Architecture

### Initialization Flow

The character base provides a structured initialization pattern that accommodates both player and AI characters with different ownership models:

```cpp
virtual void InitializeAbilityActorInfo();
```

**Purpose:** Abstract method for subclasses to implement GAS initialization sequence:
1. Resolve ASC ownership (PlayerState vs Character)  
2. Call `ASC->InitAbilityActorInfo(OwnerActor, AvatarActor)`
3. Apply default attributes
4. Grant startup abilities

### Ownership Patterns

#### Player Characters
```cpp
// ASC owned by PlayerState for persistence across respawns
void ATDPlayerCharacter::InitializeAbilityActorInfo()
{
    if (ATDPlayerState* TDPS = GetPlayerState<ATDPlayerState>())
    {
        AbilitySystemComponent = TDPS->GetAbilitySystemComponent();
        AttributeSet = TDPS->GetAttributeSet();
        
        AbilitySystemComponent->InitAbilityActorInfo(TDPS, this);
        
        // Apply default attributes and grant abilities
        DefaultAttributeInitComponent->InitializeDefaultAttributes();
        AbilityInitComponent->AddCharacterAbilities();
    }
}
```

#### AI Characters  
```cpp
// ASC owned by Character for immediate availability
void ATDEnemyCharacter::InitializeAbilityActorInfo()
{
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    
    // Apply default attributes and grant abilities
    DefaultAttributeInitComponent->InitializeDefaultAttributes();
    AbilityInitComponent->AddCharacterAbilities();
}
```

## Combat Interface Implementation

### Level Management

The `IGASCoreCombatInterface` provides level information for gameplay calculations:

```cpp
virtual int32 GetActorLevel() override;
```

**Implementation Patterns:**

**Player Characters:**
```cpp
int32 ATDPlayerCharacter::GetActorLevel()
{
    // Fetch level from PlayerState for persistence
    if (ATDPlayerState* TDPS = GetPlayerState<ATDPlayerState>())
    {
        return TDPS->GetCharacterLevel();
    }
    return 1; // Default fallback
}
```

**AI Characters:**
```cpp
int32 ATDEnemyCharacter::GetActorLevel()
{
    // AI level stored directly on character
    return EnemyCharacterLevel;
}
```

### Usage in Gameplay Systems

The level information is commonly used in:
- **Magnitude Calculation Classes (MMCs):** For level-scaled attributes
- **Damage Calculations:** Level-based damage scaling
- **Effect Potency:** Level-dependent effect strength
- **XP/Progression:** Level difference calculations

## Component Integration

### Default Attribute Initialization

`UTDDefaultAttributeInitComponent` handles applying base attribute values:

```cpp
void UTDDefaultAttributeInitComponent::InitializeDefaultAttributes()
{
    // Apply base attribute effects like Health, Mana, etc.
    if (DefaultAttributeEffects.Num() > 0)
    {
        UAbilitySystemComponent* ASC = GetOwner()->GetAbilitySystemComponent();
        for (TSubclassOf<UGameplayEffect> EffectClass : DefaultAttributeEffects)
        {
            if (EffectClass)
            {
                FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
                Context.AddSourceObject(GetOwner());
                
                FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }
}
```

### Ability Initialization

`UTDAbilityInitComponent` manages startup ability granting:

```cpp
void UTDAbilityInitComponent::AddCharacterAbilities()
{
    if (!GetOwner()->HasAuthority()) return;
    
    UAbilitySystemComponent* ASC = GetOwner()->GetAbilitySystemComponent();
    if (ASC && StartupAbilities.Num() > 0)
    {
        for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
        {
            if (AbilityClass)
            {
                FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, -1, GetOwner());
                ASC->GiveAbility(AbilitySpec);
            }
        }
    }
}
```

## Derived Class Patterns

### Player Character Implementation

```cpp
class RPG_TOPDOWN_API ATDPlayerCharacter : public ATDCharacterBase
{
    GENERATED_BODY()

public:
    ATDPlayerCharacter();
    
protected:
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;
    virtual void InitializeAbilityActorInfo() override;
    virtual int32 GetActorLevel() override;

private:
    /** Initialize input component with enhanced input bindings */
    void InitializeInputComponent();
    
    /** Bind abilities to input actions */
    void BindAbilityInputs();
};
```

**Key Responsibilities:**
- Input handling and ability activation
- PlayerState integration for GAS ownership
- UI/HUD communication via widget controllers
- Client-server synchronization for multiplayer

### Enemy Character Implementation

```cpp
class RPG_TOPDOWN_API ATDEnemyCharacter : public ATDCharacterBase, public IHighlightInterface
{
    GENERATED_BODY()

public:
    ATDEnemyCharacter();
    
protected:
    virtual void BeginPlay() override;
    virtual void InitializeAbilityActorInfo() override;
    virtual int32 GetActorLevel() override;
    
    // IHighlightInterface
    virtual void HighlightActor() override;
    virtual void UnHighlightActor() override;

private:
    /** AI-specific level (stored on character) */
    UPROPERTY(EditAnywhere, Category = "Character Class Defaults")
    int32 EnemyCharacterLevel = 1;
    
    /** Highlighting state for interaction system */
    bool bHighlighted = false;
};
```

**Key Responsibilities:**
- AI behavior integration
- Highlighting for player interaction
- Self-contained GAS ownership
- Level management for difficulty scaling

## Best Practices

### Initialization Timing

**Critical Order:**
1. Component creation (Constructor)
2. ASC/AttributeSet assignment (derived class)
3. `InitAbilityActorInfo()` call
4. Default attribute application
5. Startup ability granting

**Common Timing Points:**

| Character Type | Server | Client |
|----------------|--------|--------|
| Player | `PossessedBy()` | `OnRep_PlayerState()` |
| AI | `BeginPlay()` | N/A (server only) |

### Error Handling

```cpp
void ATDCharacterBase::InitializeAbilityActorInfo()
{
    // Validate critical components
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null for %s"), *GetName());
        return;
    }
    
    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeSet is null for %s"), *GetName());
        return;
    }
    
    // Proceed with initialization...
}
```

### Performance Considerations

```cpp
ATDCharacterBase::ATDCharacterBase()
{
    // Disable tick by default for performance
    PrimaryActorTick.bCanEverTick = false;
    
    // Create components with efficient names
    DefaultAttributeInitComponent = CreateDefaultSubobject<UTDDefaultAttributeInitComponent>("DefaultAttributeInitComponent");
    AbilityInitComponent = CreateDefaultSubobject<UTDAbilityInitComponent>("AbilityInitComponent");
    
    // Optimize weapon mesh
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
    WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponHandSocket"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
```

## Troubleshooting

### Common Issues

**ASC Not Initialized:**
```
Symptoms: Abilities don't work, attributes are default values
Causes: InitializeAbilityActorInfo() not called or called too early
Solution: Ensure proper timing based on character type
```

**Attributes Not Applied:**
```
Symptoms: Health/Mana remain at 0 or default values
Causes: DefaultAttributeInitComponent not configured or applied
Solution: Verify GameplayEffect classes are set and valid
```

**Abilities Not Granted:**
```
Symptoms: StartupAbilities array populated but abilities missing from ASC
Causes: Authority check failing or component not ready
Solution: Add HasAuthority() check and verify ASC initialization
```

**Level Always Returns 1:**
```
Symptoms: Gameplay calculations use default level value
Causes: GetActorLevel() not properly overridden in derived classes
Solution: Implement level retrieval based on character ownership pattern
```

### Debug Utilities

```cpp
// Console command to debug character state
UFUNCTION(Exec, Category = "Debug")
void DebugCharacterInfo()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Character Debug Info ==="));
    UE_LOG(LogTemp, Warning, TEXT("Character: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("Level: %d"), GetActorLevel());
    UE_LOG(LogTemp, Warning, TEXT("ASC: %s"), AbilitySystemComponent ? TEXT("Valid") : TEXT("Null"));
    UE_LOG(LogTemp, Warning, TEXT("AttributeSet: %s"), AttributeSet ? TEXT("Valid") : TEXT("Null"));
    
    if (AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Granted Abilities: %d"), AbilitySystemComponent->GetActivatableAbilities().Num());
    }
}
```

## Integration Examples

### Widget Controller Communication

```cpp
void ATDPlayerCharacter::InitializeWidgetController()
{
    if (UTDHUDWidgetController* HUDController = GetHUDWidgetController())
    {
        // Bind attribute changes to UI updates
        HUDController->BindCallbacksToDependencies();
        
        // Broadcast initial values
        HUDController->BroadcastInitialValues();
    }
}
```

### Ability Input Binding

```cpp
void ATDPlayerCharacter::BindAbilityInputs()
{
    if (UTDEnhancedInputComponent* TDInputComponent = Cast<UTDEnhancedInputComponent>(InputComponent))
    {
        // Bind ability activation inputs
        TDInputComponent->BindAbilityActions(InputConfig, this, 
            &ThisClass::AbilityInputTagPressed,
            &ThisClass::AbilityInputTagReleased,
            &ThisClass::AbilityInputTagHeld);
    }
}
```

### Combat Integration

```cpp
void ATDCharacterBase::ReceiveDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // Apply damage via gameplay effect
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(DamageInstigator);
        
        FGameplayEffectSpecHandle DamageSpec = ASC->MakeOutgoingSpec(
            DamageEffectClass, GetActorLevel(), Context);
            
        if (DamageSpec.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
        }
    }
}
```

## See Also

- [GAS Integration Overview](../gas/abilities/overview.md)
- [Character Initialization Patterns](../architecture/character-initialization.md)
- [Ability System Components](../gas/components/ability-system-component.md)
- [Attribute System](../gas/attributes/overview.md)
- [Combat Interface](combat-interface.md)