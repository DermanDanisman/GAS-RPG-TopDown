# Aura Projectile Implementation (Course Lecture)

Last updated: 2024-12-19

> **Course Context**: This document covers the projectile implementation from the Aura GAS course transcript `aura_projectile.cpp`.

## Lecture Overview

This lecture demonstrates implementing projectiles within the GAS framework, covering:
- Creating projectile actors with movement and collision
- Spawning projectiles from gameplay abilities  
- Applying damage through GameplayEffects on hit
- Network replication considerations
- Integration with the broader ability system

## Course Implementation: Aura Projectile

### Base Projectile Class Structure

Following the course pattern, the `AAuraProjectile` class extends `AActor` with these key components:

```cpp
UCLASS()
class AURA_API AAuraProjectile : public AActor
{
    GENERATED_BODY()

public:
    AAuraProjectile();

protected:
    virtual void BeginPlay() override;

    // Collision detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    class USphereComponent* Sphere;

    // Physics movement
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    class UProjectileMovementComponent* ProjectileMovement;

    // Visual mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    class UStaticMeshComponent* Mesh;
    
    // Collision callback
    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);

    // Damage effect to apply
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    // Effect application properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float DamageAmount = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float DestroyTime = 3.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bHit = false;
};
```

### Key Implementation Details

#### Constructor Setup
```cpp
AAuraProjectile::AAuraProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    SetRootComponent(Sphere);
    Sphere->SetCollisionObjectType(ECollisionChannel::ECC_Projectile);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
    Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 550.f;
    ProjectileMovement->MaxSpeed = 550.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
}
```

#### Collision and Damage Application
```cpp
void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                      bool bFromSweep, const FHitResult& SweepResult)
{
    if (DamageEffectClass && OtherActor != GetInstigator() && !bHit)
    {
        bHit = true; // Prevent multiple hits
        
        // Get target's Ability System Component
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor))
        {
            if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
            {
                // Create effect context from instigator's ASC
                if (IAbilitySystemInterface* SourceASI = Cast<IAbilitySystemInterface>(GetInstigator()))
                {
                    if (UAbilitySystemComponent* SourceASC = SourceASI->GetAbilitySystemComponent())
                    {
                        FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
                        ContextHandle.Get()->SetEffectCauser(this);
                        ContextHandle.Get()->AddSourceObject(this);

                        const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
                            DamageEffectClass, 1.f, ContextHandle);
                        
                        // Apply the effect to target
                        TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                    }
                }
            }
        }

        // Destroy projectile after hit
        SetLifeSpan(DestroyTime);
    }
}
```

### Spawning from Abilities

The course demonstrates spawning projectiles from a fireball ability:

```cpp
// In FireballSpell ability
UCLASS()
class AURA_API UFireballSpell : public UAuraProjectileSpell
{
    GENERATED_BODY()

protected:
    virtual void SpawnProjectile(const FVector& ProjectileTargetLocation) override;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fireball")
    TSubclassOf<AAuraProjectile> ProjectileClass;
};

void UFireballSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer) return;

    // Calculate spawn transform
    FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
        GetAvatarActorFromActorInfo(),
        FAuraGameplayTags::Get().CombatSocket_Weapon);
        
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

    // Spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetAvatarActorFromActorInfo();
    SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn the projectile
    AAuraProjectile* Projectile = GetWorld()->SpawnActor<AAuraProjectile>(
        ProjectileClass,
        SocketLocation,
        Rotation,
        SpawnParams);

    // Configure projectile properties
    const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
    FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
    EffectContextHandle.SetAbility(this);
    EffectContextHandle.AddSourceObject(Projectile);

    const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        DamageEffectClass,
        GetAbilityLevel(),
        EffectContextHandle);

    FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
    
    for (auto& Pair : DamageTypes)
    {
        const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
    }

    Projectile->DamageEffectSpecHandle = SpecHandle;
}
```

## Course Learning Objectives

### 1. Projectile Actor Creation
- Setting up collision components with appropriate channels
- Configuring projectile movement for desired trajectory
- Implementing visual mesh and effects

### 2. GAS Integration
- Using GameplayEffects for damage application
- Proper effect context creation and source attribution
- SetByCaller magnitude assignment for dynamic values

### 3. Network Considerations
- Server-authoritative spawning and hit detection
- Replication of projectile movement and destruction
- Authority checks for effect application

### 4. Ability System Patterns
- Spawning actors from ability execution
- Passing ability context to spawned actors
- Proper cleanup and lifecycle management

## Course Context: Why This Approach?

The Aura course emphasizes several key principles demonstrated in this projectile implementation:

### Server Authority
- All damage-dealing projectiles spawn on server only
- Hit detection and effect application happen server-side
- Clients receive replicated projectile movement for visual feedback

### GAS Integration
- Projectiles carry GameplayEffectSpecHandle for damage application
- Effects use proper context with ability and source information
- SetByCaller allows dynamic damage scaling based on ability level

### Modular Design
- Base projectile class can be specialized for different spell types
- Damage effects are configurable per projectile type
- Movement and visual properties exposed for designer iteration

## Common Course Student Mistakes

### 1. Client-Side Spawning
```cpp
// ❌ Wrong - spawns on all clients
GetWorld()->SpawnActor<AAuraProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);

// ✅ Correct - server only
const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
if (!bIsServer) return;
GetWorld()->SpawnActor<AAuraProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
```

### 2. Missing Effect Context
```cpp
// ❌ Wrong - no proper source attribution
TargetASC->ApplyGameplayEffectToSelf(DamageEffect);

// ✅ Correct - with context
FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.f, ContextHandle);
TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
```

### 3. Collision Configuration Issues
```cpp
// ❌ Wrong - may not hit intended targets
Sphere->SetCollisionResponseToAllChannels(ECR_Block);

// ✅ Correct - selective collision responses
Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
```

## Testing and Validation

### Course Testing Approach
1. **Single Player**: Verify projectile spawning and damage application
2. **Multiplayer**: Test server/client behavior and replication
3. **Edge Cases**: Multiple hits, invalid targets, network lag
4. **Performance**: Multiple projectiles, object pooling considerations

### Debug Console Commands
```
showdebug abilitysystem
showdebug collision
stat game
```

### Visual Debug Helpers
```cpp
#if !UE_BUILD_SHIPPING
// Draw debug sphere at hit location
DrawDebugSphere(GetWorld(), HitLocation, 25.f, 12, FColor::Red, false, 3.f);

// Log projectile events
UE_LOG(LogAura, Warning, TEXT("Projectile %s hit %s"), *GetName(), *OtherActor->GetName());
#endif
```

## Course Extensions and Exercises

### 1. Homing Projectiles
Modify the projectile to track the nearest enemy:
- Add target acquisition logic
- Implement steering behavior
- Balance homing strength vs. dodge-ability

### 2. Bouncing Projectiles
Create projectiles that bounce off walls:
- Configure bounce material properties
- Limit bounce count
- Apply reduced damage per bounce

### 3. AoE Projectiles
Expand to area-of-effect damage:
- Sphere overlap on hit location
- Apply effects to all targets in radius
- Different damage for direct hit vs. splash

## Next Lecture Topics

The projectile implementation leads into several advanced topics:
- **Ability Tasks**: Custom tasks for complex projectile behaviors
- **Gameplay Cues**: Visual and audio effects for projectile events
- **Attribute Sets**: Damage calculations based on caster/target attributes
- **Effect Stacking**: Multiple projectile hits and diminishing returns

## See Also

- [Projectiles in GAS Abilities](../gas/abilities/projectiles.md) - Complete implementation guide
- [Gameplay Effects](../gas/gameplay-effects.md) - Damage effect configuration
- [Ability System Overview](../gas/abilities/overview.md) - Broader ability context
- [Course Notes](../guides/course-notes.md) - Other course implementations