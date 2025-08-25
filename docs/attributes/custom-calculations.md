# Custom Calculations (MMCs) for Max Health and Max Mana

Goal: Compute derived attributes using more than AttributeBased magnitudes by mixing backing Attributes (Vigor/Intelligence) with a non-Attribute (Level) exposed via a Combat interface. We'll implement two UGameplayModMagnitudeCalculation classes (MMCs): one for Max Health and one for Max Mana, wire them into an infinite GE, and initialize vitals to full via an instant GE.

Use this when AttributeBased isn't enough (e.g., you need Level, difficulty, or equipment data not modeled as Attributes).

## MMC vs AttributeBased — choose the right tool
- AttributeBased: "Attribute X drives Attribute Y," re-evaluates automatically when captured Attributes change.
- MMC (UGameplayModMagnitudeCalculation): custom C++ returns a float. Use when you must combine Attributes with non-Attributes (Level), or express branching/curve/table-driven logic.

We will:
- Keep Level as an int32 (not an Attribute) on PlayerState for players, on Character for enemies.
- Expose Level via a lightweight ICombatInterface::GetPlayerLevel() to keep MMCs decoupled.
- Build MMCs for MaxHealth/MaxMana and use them in an infinite GE with Override.
- Initialize Health and Mana to their max values with a simple instant GE after secondary attributes are set.

## Engine Classes Deep Dive

### UGameplayEffectCalculation (Base Class)
The root class for all custom magnitude calculations in GAS. Provides the foundation for MMCs but is rarely used directly.

Key responsibilities:
- Defines the interface for custom magnitude calculations
- Manages the `RelevantAttributesToCapture` array for dependency tracking
- Provides access to the effect context and specification during evaluation

### UGameplayModMagnitudeCalculation (MMCs)
Your primary tool for custom magnitude calculations. Inherits from `UGameplayEffectCalculation` and adds the core calculation logic.

#### What RelevantAttributesToCapture Does
```cpp
// In your MMC constructor
UMMC_MaxHealth::UMMC_MaxHealth()
{
    // Register attributes this MMC depends on
    RelevantAttributesToCapture.Add(MaxHealthMMC::VigorDef);
    // GAS uses this to:
    // 1. Know when to re-evaluate this MMC (when Vigor changes)
    // 2. Capture attribute values at the right time
    // 3. Build dependency graphs for aggregator updates
}
```

The `RelevantAttributesToCapture` array serves multiple critical purposes:
- **Dependency tracking**: GAS knows this MMC depends on the captured attributes
- **Auto-recomputation**: When a captured attribute changes, infinite GEs using this MMC recalculate
- **Capture registration**: Tells the system which attribute values to snapshot/read during evaluation
- **Performance optimization**: Only captures attributes you actually need

#### How Captures Are Registered and Read
Attribute captures are defined using `FGameplayEffectAttributeCaptureDefinition`:

```cpp
// Step 1: Define the capture (usually in an anonymous namespace)
namespace MaxHealthMMC
{
    static FGameplayEffectAttributeCaptureDefinition VigorDef(
        UYourAttributeSet::GetVigorAttribute(),           // Which attribute
        EGameplayEffectAttributeCaptureSource::Target,    // From Source or Target
        false                                             // bSnapshot
    );
}

// Step 2: Register in constructor
RelevantAttributesToCapture.Add(MaxHealthMMC::VigorDef);

// Step 3: Read during calculation
float Vigor = 0.f;
GetCapturedAttributeMagnitude(MaxHealthMMC::VigorDef, Spec, Params, Vigor);
```

## CalculateBaseMagnitude_Implementation Lifecycle

### When It Is Called
The `CalculateBaseMagnitude_Implementation` function is invoked:

1. **Initial effect application**: When a GE using this MMC is first applied
2. **Dependency changes**: When any captured attribute changes (if bSnapshot=false)
3. **Manual recomputation**: When the effect is manually recalculated or reapplied
4. **Aggregator updates**: During aggregator re-evaluation cycles
5. **Tag changes**: When source/target tags change and affect evaluation parameters

**Not called for**:
- External dependency changes (like Level) unless you manually trigger recomputation
- SetByCaller value changes on existing effects
- Context changes after effect creation

### How The Returned Base Value Is Transformed

Your MMC's returned value goes through additional transformations via `FCustomCalculationBasedFloat`:

```cpp
// Your MMC returns: BaseValue = 150.0f
float CalculateBaseMagnitude_Implementation(...) const
{
    return 150.0f;  // This is the "base" value
}

// GAS then applies FCustomCalculationBasedFloat transform:
// FinalMagnitude = (BaseValue + PreMultiplyAdditiveValue) * MultiplierValue + PostMultiplyAdditiveValue
//
// If your GE modifier has:
// - PreMultiplyAdditiveValue = 10.0f
// - MultiplierValue = 1.2f  
// - PostMultiplyAdditiveValue = 5.0f
//
// Result: (150.0 + 10.0) * 1.2 + 5.0 = 160.0 * 1.2 + 5.0 = 192.0 + 5.0 = 197.0f
```

This allows designers to tune MMC output without changing code:
- **PreMultiplyAdditiveValue**: Flat adjustment before scaling
- **MultiplierValue**: Percentage scaling (1.0 = no change)
- **PostMultiplyAdditiveValue**: Flat adjustment after scaling

### Recomputation Triggers

MMCs automatically recompute when:
- **Captured attributes change** (if bSnapshot=false)
- **Effect is reapplied** (remove and reapply the GE)
- **Manual refresh** via `ASC->GetActiveEffects().SetBaseAttributeValueFromSource()`

MMCs do **not** automatically recompute when:
- **External dependencies change** (Level, equipment, etc.)
- **SetByCaller values change** on existing effects
- **Non-captured game state changes**

For external dependencies, you must choose a recompute strategy:
1. **Reapply the GE**: Remove and reapply when dependencies change
2. **Use GetExternalModifierDependencyMulticast**: Advanced dependency tracking
3. **Model as Attributes**: Convert external data to attributes for auto-tracking

## Attribute Capture Mechanics

### AttributeToCapture
Specifies which attribute to read during MMC evaluation. Must be a valid `FGameplayAttribute` from an AttributeSet.

```cpp
// Captures the Vigor attribute
FGameplayEffectAttributeCaptureDefinition VigorDef(
    UYourAttributeSet::GetVigorAttribute(),  // The specific attribute
    EGameplayEffectAttributeCaptureSource::Target,
    false
);
```

### AttributeSource (Source vs Target)
Determines whose attribute value to capture:

```cpp
// Target: Capture from the actor receiving the effect
EGameplayEffectAttributeCaptureSource::Target

// Source: Capture from the actor causing/applying the effect  
EGameplayEffectAttributeCaptureSource::Source
```

**When to use each**:
- **Target**: Most common. Read attributes from the actor being affected (e.g., their Vigor for their MaxHealth)
- **Source**: Read from the effect's originator (e.g., attacker's Strength affecting damage calculation)

### bSnapshot Semantics
Controls when attribute values are captured:

```cpp
// bSnapshot = true: Capture at effect spec creation time
static FGameplayEffectAttributeCaptureDefinition VigorDef(
    UYourAttributeSet::GetVigorAttribute(),
    EGameplayEffectAttributeCaptureSource::Target,
    true  // Snapshot = freeze value when spec is created
);

// bSnapshot = false: Capture at evaluation time (live)
static FGameplayEffectAttributeCaptureDefinition VigorDef(
    UYourAttributeSet::GetVigorAttribute(), 
    EGameplayEffectAttributeCaptureSource::Target,
    false  // Live = read current value during each evaluation
);
```

**Use cases**:
- **bSnapshot = false** (recommended for derived attributes): Live tracking, updates when source attribute changes
- **bSnapshot = true** (useful for buffs/debuffs): Fixed value, immune to source changes during effect lifetime

### FAggregatorEvaluateParameters and Source/Target Tags

The `FAggregatorEvaluateParameters` structure controls how attribute values are evaluated with respect to gameplay tags:

```cpp
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Build evaluation parameters from captured tags
    FAggregatorEvaluateParameters Params;
    Params.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    Params.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    // Tags affect how modifiers are evaluated
    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(MaxHealthMMC::VigorDef, Spec, Params, Vigor);
    
    // The returned Vigor value respects tag-based modifier filtering
    // E.g., a "Weakness" tag might reduce Vigor's effective value
}
```

**How tags affect evaluation**:
- Modifiers on attributes can have `RequiredTags` and `IgnoreTags` 
- `Params.SourceTags/TargetTags` determine which modifiers apply during capture
- Allows conditional attribute calculations (e.g., "Berserker" mode affects Strength differently)

**Best practices**:
- Always populate `SourceTags` and `TargetTags` for consistent evaluation
- Use `GetAggregatedTags()` to include all relevant tags from the effect context
- Consider tag implications when designing conditional modifiers

## External Dependencies: Beyond Attribute Captures

MMCs often need data that isn't modeled as attributes. This section covers patterns, tradeoffs, and strategies for handling external dependencies.

### Patterns for Non-Attribute Inputs

#### Level via ICombatInterface (Current Pattern)
```cpp
static int32 ResolveLevel(const FGameplayEffectSpec& Spec)
{
    int32 Level = 1;
    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();

    // Primary: try SourceObject
    if (const UObject* SourceObj = Ctx.GetSourceObject())
    {
        if (SourceObj->Implements<UCombatInterface>())
        {
            Level = ICombatInterface::Execute_GetActorLevel(SourceObj);
        }
    }
    
    // Fallback: try OriginalInstigator
    if (Level <= 0)
    {
        if (const AActor* Instigator = Ctx.GetOriginalInstigator())
        {
            if (Instigator->Implements<UCombatInterface>())
            {
                Level = ICombatInterface::Execute_GetActorLevel(const_cast<AActor*>(Instigator));
            }
        }
    }
    
    return FMath::Max(1, Level);
}
```

#### Equipment/Gear Data via Context
```cpp
float GetEquipmentBonus(const FGameplayEffectSpec& Spec) const
{
    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();
    
    if (const UObject* SourceObj = Ctx.GetSourceObject())
    {
        if (const IEquipmentInterface* Equipment = Cast<IEquipmentInterface>(SourceObj))
        {
            return Equipment->GetWeaponDamageMultiplier();
        }
    }
    return 1.0f;
}
```

#### Game Mode/Difficulty Settings
```cpp
float GetDifficultyMultiplier(const FGameplayEffectSpec& Spec) const
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(Spec.GetContext().GetSourceObject()))
    {
        if (const AGameModeBase* GameMode = World->GetAuthGameMode())
        {
            if (const IDifficultyInterface* Difficulty = Cast<IDifficultyInterface>(GameMode))
            {
                return Difficulty->GetAttributeMultiplier();
            }
        }
    }
    return 1.0f;
}
```

### Tradeoffs of External Dependencies

| Pattern | Pros | Cons | Best For |
|---------|------|------|----------|
| **Interface Access** | Flexible, decoupled, testable | Manual recompute needed | Level, character stats |
| **Context SourceObject** | Direct access, rich data | Tight coupling, null checks | Equipment, temporary state |
| **Global/Singleton** | Always available, cached | Hard to test, synchronization issues | Game settings, difficulty |
| **SetByCaller** | Event-driven, GAS-native | Must pass at application time | Dynamic values, UI inputs |

### Three Recompute Strategies

#### Strategy 1: Reapply GE (Recommended for Level)
```cpp
// In your level-up logic
void AYourCharacter::OnLevelChanged()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // Remove existing secondary attributes effect
        ASC->RemoveActiveGameplayEffectBySourceEffect(SecondaryAttributesGE, ASC);
        
        // Reapply with updated level
        ApplyEffectToSelf(SecondaryAttributesGE, GetActorLevel());
    }
}
```

**Pros**: Simple, reliable, works with all dependency types  
**Cons**: Performance cost, effect stack changes, momentary gaps  
**Use when**: Dependencies change infrequently (level-ups, major state changes)

#### Strategy 2: GetExternalModifierDependencyMulticast (Advanced)
```cpp
// In your MMC
UMMC_MaxHealth::UMMC_MaxHealth()
{
    RelevantAttributesToCapture.Add(MaxHealthMMC::VigorDef);
    
    // Register for external dependency notifications
    bAllowNonNetAuthorityDependencyRegistration = false; // See below
}

// In your dependency source (e.g., PlayerState)
void AYourPlayerState::SetLevel(int32 NewLevel)
{
    if (HasAuthority() && Level != NewLevel)
    {
        Level = NewLevel;
        
        // Notify all MMCs that depend on level
        GetExternalModifierDependencyMulticast().Broadcast();
    }
}
```

**Pros**: Automatic recomputation, fine-grained control, efficient  
**Cons**: Complex setup, networking considerations, hard to debug  
**Use when**: Dependencies change frequently, performance is critical

#### Strategy 3: Model as Attribute (When Appropriate)
```cpp
// Convert Level to an attribute
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level)
FGameplayAttributeData Level;

// Use AttributeBased magnitude instead of MMC
// Max Health = AttributeBased(Vigor) + AttributeBased(Level)
```

**Pros**: Full GAS integration, automatic recomputation, networking support  
**Cons**: More attributes to manage, may not fit domain model  
**Use when**: Data behaves like an attribute, needs full GAS features

### Advanced Flag: bAllowNonNetAuthorityDependencyRegistration

```cpp
// In MMC constructor
UMMC_MaxHealth::UMMC_MaxHealth()
{
    RelevantAttributesToCapture.Add(MaxHealthMMC::VigorDef);
    
    // Default: false (recommended)
    bAllowNonNetAuthorityDependencyRegistration = false;
}
```

**When false (recommended)**:
- Only server can register external dependencies
- Prevents client-side dependency registration
- Ensures server-authoritative calculations
- Avoids prediction/synchronization issues

**When true (advanced use cases)**:
- Allows client-side dependency registration
- Enables client prediction of external changes
- Requires careful synchronization between client/server
- Can cause desync if not handled properly

**Constraints**:
- Multiplayer games should generally keep this false
- Single-player games can safely use true
- Prediction-heavy games need custom synchronization logic

## SetByCaller Inputs: Event-Driven Values

SetByCaller provides a way to pass dynamic values to MMCs at effect application time, useful for UI-driven inputs or event-specific parameters.

### Reading SetByCaller by Tag
```cpp
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Define your SetByCaller tag (shared with application code)
    static const FGameplayTag LevelOverrideTag = FGameplayTag::RequestGameplayTag("SetByCaller.LevelOverride");
    
    // Try to read the SetByCaller value
    float LevelOverride = 0.0f;
    if (Spec.HasSetByCallerWithTag(LevelOverrideTag))
    {
        LevelOverride = Spec.GetSetByCallerMagnitude(LevelOverrideTag, false, 0.0f);
    }
    
    // Use override if provided, otherwise fall back to interface
    const int32 Level = (LevelOverride > 0.0f) ? 
        FMath::RoundToInt(LevelOverride) : 
        ResolveLevel(Spec);
        
    // Continue with calculation...
    return BaseHealth + (Level * LevelMultiplier);
}
```

### Reading SetByCaller by Name (Alternative)
```cpp
// Less preferred due to string comparisons and magic constants
float LevelOverride = Spec.GetSetByCallerMagnitude(FName("LevelOverride"), false, 0.0f);
```

### When to Prefer SetByCaller over Captures

**Use SetByCaller when**:
- Values are event-specific (damage from UI, temporary modifiers)
- Data comes from non-GAS systems (inventory, progression)
- Values are computed at application time (random rolls, calculations)
- You need guaranteed synchronization with effect timing

**Use Captures when**:
- Values are persistent character attributes
- You want automatic recomputation on changes
- Values are part of the core attribute system
- Dependencies are well-modeled as attributes

### SetByCaller Application Example
```cpp
// In your effect application code
void ApplyDamageWithBonus(AActor* Target, float BaseDamage, float BonusMultiplier)
{
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    Context.AddSourceObject(this);
    
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageGE, 1.0f, Context);
    if (SpecHandle.IsValid())
    {
        // Pass bonus multiplier via SetByCaller
        static const FGameplayTag BonusTag = FGameplayTag::RequestGameplayTag("SetByCaller.BonusMultiplier");
        SpecHandle.Data->SetSetByCallerMagnitude(BonusTag, BonusMultiplier);
        
        // Apply to target
        ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, Target->GetAbilitySystemComponent());
    }
}
```

## Context Details and Best Practices

### Understanding the Context Hierarchy
The `FGameplayEffectContextHandle` provides multiple ways to access the source of an effect:

```cpp
const FGameplayEffectContextHandle& Ctx = Spec.GetContext();

// SourceObject: Explicitly set object (your primary data source)
const UObject* SourceObj = Ctx.GetSourceObject();

// Instigator: The actor who initiated the effect (may be different from SourceObject)
const AActor* Instigator = Ctx.GetInstigator();

// OriginalInstigator: The original actor in a chain of effects
const AActor* OriginalInstigator = Ctx.GetOriginalInstigator();
```

### SourceObject (Primary Data Source)
**What it is**: An explicitly set UObject that carries data needed by the effect.

**Common patterns**:
```cpp
// Pattern 1: Self-application (character applying effect to themselves)
FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
Context.AddSourceObject(this);  // 'this' = the character

// Pattern 2: Equipment as source (weapon affecting damage)
Context.AddSourceObject(CurrentWeapon);

// Pattern 3: Ability as source (ability-specific calculations)
Context.AddSourceObject(CastingAbility);
```

### Instigator vs OriginalInstigator
```cpp
// Example: Player -> Pet -> Target damage chain

// When Player commands Pet to attack Target:
// - OriginalInstigator = Player (who started the chain)
// - Instigator = Pet (who directly applied the effect)
// - SourceObject = Pet's weapon/ability (data source)

static int32 ResolveLevel(const FGameplayEffectSpec& Spec)
{
    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();
    
    // Try SourceObject first (most specific)
    if (const UObject* SourceObj = Ctx.GetSourceObject())
    {
        if (SourceObj->Implements<UCombatInterface>())
        {
            return ICombatInterface::Execute_GetActorLevel(SourceObj);
        }
    }
    
    // Fallback to OriginalInstigator (ultimate source)
    if (const AActor* OriginalInstigator = Ctx.GetOriginalInstigator())
    {
        if (OriginalInstigator->Implements<UCombatInterface>())
        {
            return ICombatInterface::Execute_GetActorLevel(const_cast<AActor*>(OriginalInstigator));
        }
    }
    
    // Last resort: direct instigator
    if (const AActor* Instigator = Ctx.GetInstigator())
    {
        if (Instigator->Implements<UCombatInterface>())
        {
            return ICombatInterface::Execute_GetActorLevel(const_cast<AActor*>(Instigator));
        }
    }
    
    return 1; // Default fallback
}
```

### Spec Level
The `FGameplayEffectSpec` has its own level, separate from character level:

```cpp
// Access spec level
float SpecLevel = Spec.GetLevel();

// Spec level vs Character level:
// - Spec level: The level at which this specific effect was applied
// - Character level: The character's current level (may have changed since application)

// Use spec level when: Effect should maintain its original power
// Use character level when: Effect should scale with current character power
```

### Best Practices for Setting SourceObject

#### Always Set SourceObject for MMCs
```cpp
// ❌ Wrong: No SourceObject set
void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GEClass)
{
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    // Missing: Context.AddSourceObject(this);
    
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, 1.0f, Context);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
}

// ✅ Correct: SourceObject provides interface access
void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GEClass)
{
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    Context.AddSourceObject(this); // MMCs can now access ICombatInterface
    
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, 1.0f, Context);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
}
```

#### Set SourceObject in Ability System Components
```cpp
// In your ASC initialization/effect application wrapper
void UCoreAbilitySystemComponent::ApplyGameplayEffectToSelf(
    TSubclassOf<UGameplayEffect> GameplayEffectClass, 
    float Level)
{
    FGameplayEffectContextHandle EffectContext = MakeEffectContext();
    
    // Ensure MMCs can access the avatar actor's interfaces
    EffectContext.AddSourceObject(GetAvatarActor());
    
    if (EffectContext.Get())
    {
        EffectContext.Get()->SetEffectCauser(GetAvatarActor());
    }
    
    const FGameplayEffectSpecHandle SpecHandle = 
        MakeOutgoingSpec(GameplayEffectClass, Level, EffectContext);
        
    if (SpecHandle.IsValid())
    {
        ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
    }
}
```

#### Handle Null SourceObject Gracefully
```cpp
// Defensive programming in MMCs
static int32 ResolveLevel(const FGameplayEffectSpec& Spec, int32 DefaultLevel = 1)
{
    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();
    
    if (const UObject* SourceObj = Ctx.GetSourceObject())
    {
        if (SourceObj->Implements<UCombatInterface>())
        {
            const int32 Level = ICombatInterface::Execute_GetActorLevel(SourceObj);
            if (Level > 0)
            {
                return Level;
            }
        }
    }
    
    // Comprehensive fallback chain
    if (const AActor* OriginalInstigator = Ctx.GetOriginalInstigator())
    {
        if (OriginalInstigator->Implements<UCombatInterface>())
        {
            const int32 Level = ICombatInterface::Execute_GetActorLevel(const_cast<AActor*>(OriginalInstigator));
            if (Level > 0)
            {
                return Level;
            }
        }
    }
    
    // Log warning for debugging
    UE_LOG(LogTemp, Warning, TEXT("MMC could not resolve Level from context, using default: %d"), DefaultLevel);
    return DefaultLevel;
}

## Networking & Authority

### Server Authoritative Evaluation
MMCs always execute on the server in multiplayer games, ensuring consistent and authoritative calculations.

**Key principles**:
- Server calculates the final magnitude
- Clients receive the result via attribute replication
- No client-side MMC execution (prevents cheating)
- Context data must be available on the server

```cpp
// Server execution flow:
// 1. Client requests effect application (or server initiates)
// 2. Server creates effect spec and context
// 3. Server calls MMC::CalculateBaseMagnitude_Implementation
// 4. Server applies the calculated magnitude
// 5. Modified attributes replicate to clients
```

### Client Prediction Caveats
While MMCs themselves don't run on clients, their effects can impact prediction:

**Immediate effects**:
```cpp
// Server calculates damage via MMC
float DamageMMC::CalculateBaseMagnitude_Implementation(...) const
{
    // Complex calculation combining multiple attributes and level
    return BaseDamage + (AttackerStrength * 1.5f) + (Level * 2.0f);
}

// Client prediction challenges:
// - Client doesn't know server's exact calculation
// - Prediction must estimate or wait for server response
// - UI updates may stutter when server result arrives
```

**Solutions for prediction**:
1. **Conservative prediction**: Use simplified client-side estimates
2. **Deferred UI updates**: Wait for server confirmation before updating UI
3. **Hybrid approach**: Show estimated values with "pending" indicators

### Why External Dependency Registration Is Usually Disabled

The `bAllowNonNetAuthorityDependencyRegistration = false` setting (default) prevents client-side issues:

```cpp
// Problems with client-side external dependencies:
void OnLevelChanged()
{
    if (bAllowNonNetAuthorityDependencyRegistration == true)
    {
        // ❌ Client might trigger recomputation based on predicted level
        // ❌ Server has different level value
        // ❌ Results in client/server desync
        GetExternalModifierDependencyMulticast().Broadcast();
    }
}

// Safe approach (default behavior):
void OnLevelChanged()
{
    if (HasAuthority()) // Only server
    {
        // ✅ Server triggers recomputation with authoritative level
        // ✅ Results replicate to clients
        GetExternalModifierDependencyMulticast().Broadcast();
    }
}
```

**Networking best practices**:
- Keep external dependency changes server-authoritative
- Replicate dependency data (like Level) properly
- Use `HasAuthority()` checks before triggering recomputation
- Prefer reapplying effects over complex dependency tracking

## Performance Guidelines

### Keep MMCs Allocation-Free
MMCs are called frequently and should avoid memory allocations:

```cpp
// ❌ Avoid: Allocations in hot path
float BadMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    TArray<float> TempValues; // Heap allocation
    FString DebugString = TEXT("Calculating..."); // String allocation
    
    // Calculations that create temporary objects
    for (int32 i = 0; i < 10; ++i)
    {
        TempValues.Add(GetSomeValue(i)); // More allocations
    }
    
    return TempValues[0];
}

// ✅ Good: Stack-only calculations
float GoodMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Use stack variables
    float Result = 0.0f;
    float TempValue = 0.0f;
    
    // Direct calculations
    GetCapturedAttributeMagnitude(VigorDef, Spec, Params, TempValue);
    Result += TempValue * 2.5f;
    
    const int32 Level = ResolveLevel(Spec);
    Result += Level * 10.0f;
    
    return Result;
}
```

### Avoid Blocking Operations
MMCs should complete quickly and never block:

```cpp
// ❌ Never do this: Blocking operations
float BadMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // File I/O - blocks the main thread
    FString ConfigValue;
    FFileHelper::LoadFileToString(ConfigValue, TEXT("config.txt"));
    
    // Network requests - can timeout
    UMyHttpService::Get()->GetPlayerDataSync(PlayerId);
    
    // Database queries - unpredictable latency
    UMyDatabase::Get()->QueryPlayerLevel(PlayerId);
    
    return 0.0f; // This might never execute
}

// ✅ Good: Immediate calculations only
float GoodMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Only use readily available data
    float Vigor = 0.0f;
    GetCapturedAttributeMagnitude(VigorDef, Spec, Params, Vigor);
    
    // Interface calls should be fast
    const int32 Level = ResolveLevel(Spec);
    
    // Simple mathematical operations
    return 80.0f + (Vigor * 2.5f) + (Level * 10.0f);
}
```

### Prefer ScalableFloats/Curves for Tuning
Instead of hardcoding constants, use data-driven approaches:

```cpp
// ❌ Hardcoded: Requires code changes to tune
float HardcodedMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    const float BaseHealth = 80.0f;          // Hardcoded
    const float VigorMultiplier = 2.5f;      // Hardcoded  
    const float LevelMultiplier = 10.0f;     // Hardcoded
    
    // ... calculation
}

// ✅ Data-driven: Designers can tune without code changes
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
FScalableFloat BaseHealthValue;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")  
FScalableFloat VigorMultiplierValue;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
FScalableFloat LevelMultiplierValue;

float ScalableMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    const float Level = static_cast<float>(ResolveLevel(Spec));
    
    const float BaseHealth = BaseHealthValue.GetValueAtLevel(Level);
    const float VigorMultiplier = VigorMultiplierValue.GetValueAtLevel(Level);
    const float LevelMultiplier = LevelMultiplierValue.GetValueAtLevel(Level);
    
    // ... calculation using scalable values
}
```

**Performance tips**:
- Profile MMCs in your game's performance tests
- Use `SCOPE_CYCLE_COUNTER` for measurement
- Avoid virtual function calls in tight loops
- Cache expensive calculations when possible
- Consider pooling temporary objects if allocations are unavoidable

## Robust MMC Template

Here's a comprehensive MMC template that demonstrates best practices for this repository:

### Header File (UMMC_RobustTemplate.h)
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "UMMC_RobustTemplate.generated.h"

/**
 * Robust MMC template demonstrating:
 * - Static/lazy capture definitions
 * - Tag-aware evaluation
 * - Safe ResolveLevel fallback chain
 * - Optional SetByCaller integration
 * - Configurable rounding policy
 * - Performance-conscious implementation
 */
UCLASS()
class GASCORE_API UMMC_RobustTemplate : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()

public:
    UMMC_RobustTemplate();
    
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
    // Scalable parameters for designer tuning
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
    FScalableFloat BaseValue;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
    FScalableFloat PrimaryAttributeMultiplier;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")  
    FScalableFloat SecondaryAttributeMultiplier;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
    FScalableFloat LevelMultiplier;

    // Optional SetByCaller override tag
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
    FGameplayTag SetByCallerOverrideTag;
    
    // Rounding policy
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters")
    bool bRoundFinalValue;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMC Parameters", 
              meta = (EditCondition = "bRoundFinalValue"))
    bool bRoundUp; // If false, rounds to nearest

private:
    // Safe level resolution with fallback chain
    int32 ResolveLevel(const FGameplayEffectSpec& Spec, int32 DefaultLevel = 1) const;
    
    // Optional SetByCaller value extraction
    float GetSetByCallerOverride(const FGameplayEffectSpec& Spec) const;
    
    // Apply rounding policy to final result
    float ApplyRounding(float Value) const;
};
```

### Implementation File (UMMC_RobustTemplate.cpp)
```cpp
#include "UMMC_RobustTemplate.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "CombatInterface.h"
#include "AttributeSets/CoreAttributeSet.h"

// Static/lazy capture definitions - initialized once and reused
namespace RobustTemplateMMC
{
    // Primary attribute capture (e.g., Vigor for MaxHealth)
    static FGameplayEffectAttributeCaptureDefinition PrimaryAttributeDef(
        UCoreAttributeSet::GetVigorAttribute(),
        EGameplayEffectAttributeCaptureSource::Target,
        false // bSnapshot = false for live tracking
    );
    
    // Secondary attribute capture (e.g., Endurance for additional scaling)
    static FGameplayEffectAttributeCaptureDefinition SecondaryAttributeDef(
        UCoreAttributeSet::GetEnduranceAttribute(), 
        EGameplayEffectAttributeCaptureSource::Target,
        false // bSnapshot = false for live tracking
    );
}

UMMC_RobustTemplate::UMMC_RobustTemplate()
{
    // Register attribute dependencies for auto-recomputation
    RelevantAttributesToCapture.Add(RobustTemplateMMC::PrimaryAttributeDef);
    RelevantAttributesToCapture.Add(RobustTemplateMMC::SecondaryAttributeDef);
    
    // Initialize default scalable values
    BaseValue.Value = 80.0f;
    PrimaryAttributeMultiplier.Value = 2.5f;
    SecondaryAttributeMultiplier.Value = 1.0f;
    LevelMultiplier.Value = 10.0f;
    
    // Default SetByCaller tag (can be overridden in Blueprint)
    SetByCallerOverrideTag = FGameplayTag::RequestGameplayTag("SetByCaller.OverrideValue");
    
    // Default rounding policy
    bRoundFinalValue = true;
    bRoundUp = false; // Round to nearest
    
    // Networking: Server-authoritative external dependencies only
    bAllowNonNetAuthorityDependencyRegistration = false;
}

float UMMC_RobustTemplate::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Tag-aware evaluation parameters
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    // Check for SetByCaller override first
    const float SetByCallerOverride = GetSetByCallerOverride(Spec);
    if (SetByCallerOverride > 0.0f)
    {
        return ApplyRounding(SetByCallerOverride);
    }
    
    // Capture primary attribute with tag-aware evaluation
    float PrimaryAttributeValue = 0.0f;
    GetCapturedAttributeMagnitude(RobustTemplateMMC::PrimaryAttributeDef, Spec, EvaluationParameters, PrimaryAttributeValue);
    PrimaryAttributeValue = FMath::Max(0.0f, PrimaryAttributeValue); // Ensure non-negative
    
    // Capture secondary attribute with tag-aware evaluation  
    float SecondaryAttributeValue = 0.0f;
    GetCapturedAttributeMagnitude(RobustTemplateMMC::SecondaryAttributeDef, Spec, EvaluationParameters, SecondaryAttributeValue);
    SecondaryAttributeValue = FMath::Max(0.0f, SecondaryAttributeValue); // Ensure non-negative
    
    // Resolve level with safe fallback chain
    const int32 Level = ResolveLevel(Spec);
    const float LevelFloat = static_cast<float>(Level);
    
    // Calculate magnitude using scalable values
    const float BaseContribution = BaseValue.GetValueAtLevel(LevelFloat);
    const float PrimaryContribution = PrimaryAttributeValue * PrimaryAttributeMultiplier.GetValueAtLevel(LevelFloat);
    const float SecondaryContribution = SecondaryAttributeValue * SecondaryAttributeMultiplier.GetValueAtLevel(LevelFloat);
    const float LevelContribution = LevelFloat * LevelMultiplier.GetValueAtLevel(LevelFloat);
    
    const float FinalValue = BaseContribution + PrimaryContribution + SecondaryContribution + LevelContribution;
    
    return ApplyRounding(FinalValue);
}

int32 UMMC_RobustTemplate::ResolveLevel(const FGameplayEffectSpec& Spec, int32 DefaultLevel) const
{
    const FGameplayEffectContextHandle& Context = Spec.GetContext();
    
    // Primary: Try SourceObject (most reliable)
    if (const UObject* SourceObj = Context.GetSourceObject())
    {
        if (SourceObj->Implements<UCombatInterface>())
        {
            const int32 Level = ICombatInterface::Execute_GetActorLevel(SourceObj);
            if (Level > 0)
            {
                return Level;
            }
        }
    }
    
    // Secondary: Try OriginalInstigator (ultimate source)
    if (const AActor* OriginalInstigator = Context.GetOriginalInstigator())
    {
        if (OriginalInstigator->Implements<UCombatInterface>())
        {
            const int32 Level = ICombatInterface::Execute_GetActorLevel(const_cast<AActor*>(OriginalInstigator));
            if (Level > 0)
            {
                return Level;
            }
        }
    }
    
    // Tertiary: Try direct Instigator
    if (const AActor* Instigator = Context.GetInstigator())
    {
        if (Instigator->Implements<UCombatInterface>())
        {
            const int32 Level = ICombatInterface::Execute_GetActorLevel(const_cast<AActor*>(Instigator));
            if (Level > 0)
            {
                return Level;
            }
        }
    }
    
    // Fallback: Use Spec level if available
    const float SpecLevel = Spec.GetLevel();
    if (SpecLevel > 0.0f)
    {
        return FMath::RoundToInt(SpecLevel);
    }
    
    // Ultimate fallback: Default level
    UE_LOG(LogTemp, Warning, TEXT("[%s] Could not resolve level from any source, using default: %d"), 
           *GetClass()->GetName(), DefaultLevel);
    return DefaultLevel;
}

float UMMC_RobustTemplate::GetSetByCallerOverride(const FGameplayEffectSpec& Spec) const
{
    if (!SetByCallerOverrideTag.IsValid())
    {
        return 0.0f;
    }
    
    if (Spec.HasSetByCallerWithTag(SetByCallerOverrideTag))
    {
        return Spec.GetSetByCallerMagnitude(SetByCallerOverrideTag, false, 0.0f);
    }
    
    return 0.0f;
}

float UMMC_RobustTemplate::ApplyRounding(float Value) const
{
    if (!bRoundFinalValue)
    {
        return Value;
    }
    
    return bRoundUp ? FMath::CeilToFloat(Value) : FMath::RoundToFloat(Value);
}
```

### Key Features of This Template

**Static/Lazy Captures**:
- Captures defined once in anonymous namespace
- Reused across all instances for performance
- Clear separation between capture definitions and usage

**Tag-Aware Evaluation**:
- Uses `FAggregatorEvaluateParameters` with captured tags
- Respects tag-based modifier filtering
- Consistent evaluation context throughout

**Safe ResolveLevel Fallback**:
- Primary: `SourceObject` (most reliable for interface access)
- Secondary: `OriginalInstigator` (ultimate source in effect chains)  
- Tertiary: `Instigator` (direct effect applicator)
- Quaternary: `Spec.GetLevel()` (effect-specific level)
- Fallback: Default value with warning logging

**Optional SetByCaller Blend-In**:
- Configurable override tag via Blueprint
- Complete override behavior (not additive)
- Graceful handling when tag is invalid or missing

**Optional Rounding Policy**:
- Designer-configurable rounding behavior
- Choice between round-to-nearest and round-up
- Applied consistently to final result

**Performance Optimizations**:
- No heap allocations in hot path
- Minimal virtual function calls  
- Early returns for override cases
- Efficient attribute value clamping

## Design: Level placement + interface
- Player-controlled: Level lives on PlayerState, replicated with RepNotify.
- AI enemies: Level lives on the Character (non-replicated; server drives authoritative logic).
- A shared Combat interface abstracts "what's the level?" so MMCs don't depend on concrete classes.

### PlayerState (replicated Level with getter)
```cpp
// AuraPlayerState.h (excerpt)
UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level, Category="Level")
int32 Level = 1;

UFUNCTION()
void OnRep_Level(int32 OldLevel);

UFUNCTION(BlueprintCallable, Category="Level")
void SetLevel(int32 NewLevel);

FORCEINLINE int32 GetPlayerLevel() const { return Level; }
```
```cpp
// AuraPlayerState.cpp (excerpt)
#include "Net/UnrealNetwork.h"

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out);
    DOREPLIFETIME(AAuraPlayerState, Level);
}

void AAuraPlayerState::SetLevel(int32 NewLevel)
{
    if (HasAuthority())
    {
        const int32 Old = Level;
        Level = FMath::Max(1, NewLevel);
        OnRep_Level(Old);
    }
}

void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
    // See "Recompute on Level change" for how/when we refresh derived values.
}
```

### Enemy Character (server-only Level)
```cpp
// AuraEnemy.h (excerpt)
protected:
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
int32 Level = 1; // Not replicated (server authoritative)
```

### Combat interface (decoupling)
```cpp
// CombatInterface.h (minimal)
UINTERFACE(BlueprintType)
class UCombatInterface : public UInterface { GENERATED_BODY() };

class ICombatInterface
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat")
    int32 GetPlayerLevel() const; // Name avoids clashing with AActor::GetLevel()
};
```
Implement on:
- AuraEnemy: return its Level.
- AuraCharacter (player): fetch PlayerState and return PlayerState->GetPlayerLevel().
- Optionally implement on other combatants.

### Ensure SourceObject is set when applying GEs
MMCs will query Level via the effect context's SourceObject. When applying effects, set the source object:
```cpp
// In AuraCharacterBase::ApplyEffectToSelf(...)
FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
ContextHandle.AddSourceObject(this); // Provide self as the interface carrier
// build spec with ContextHandle and Apply
```

## MMC implementation
Each MMC captures a backing Attribute and queries Level via the interface.

Common pieces:
```cpp
// Includes you'll typically need
#include "GameplayModMagnitudeCalculation.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "CombatInterface.h"
#include "YourProject/AttributeSets/YourAttributeSet.h" // Replace with your set
```

### UMMC_MaxHealth (captures Vigor)
```cpp
// MMC_MaxHealth.h
UCLASS()
class UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()
public:
    UMMC_MaxHealth();
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
```
```cpp
// MMC_MaxHealth.cpp
namespace MaxHealthMMC
{
    static FGameplayEffectAttributeCaptureDefinition VigorDef(
        UYourAttributeSet::GetVigorAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
}

UMMC_MaxHealth::UMMC_MaxHealth()
{
    RelevantAttributesToCapture.Add(MaxHealthMMC::VigorDef);
}

static int32 ResolveLevel(const FGameplayEffectSpec& Spec)
{
    int32 Level = 1;
    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();

    if (const UObject* SourceObj = Ctx.GetSourceObject())
    {
        Level = ICombatInterface::Execute_GetPlayerLevel(SourceObj);
    }
    if (Level <= 0)
    {
        if (const AActor* Instigator = Ctx.GetOriginalInstigator())
        {
            Level = ICombatInterface::Execute_GetPlayerLevel(const_cast<AActor*>(Instigator));
        }
    }
    return FMath::Max(1, Level);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters Params;
    Params.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    Params.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(MaxHealthMMC::VigorDef, Spec, Params, Vigor);
    Vigor = FMath::Max(0.f, Vigor);

    const int32 Level = ResolveLevel(Spec);

    const float Base = 80.f;
    const float FromVigor = 2.5f * Vigor;
    const float FromLevel = 10.f * Level;
    return Base + FromVigor + FromLevel;
}
```

### UMMC_MaxMana (captures Intelligence)
```cpp
// MMC_MaxMana.h/.cpp (differences)
namespace MaxManaMMC
{
    static FGameplayEffectAttributeCaptureDefinition IntelligenceDef(
        UYourAttributeSet::GetIntelligenceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
}

// ctor
RelevantAttributesToCapture.Add(MaxManaMMC::IntelligenceDef);

// CalculateBaseMagnitude_Implementation body (key lines)
float Intelligence = 0.f;
GetCapturedAttributeMagnitude(MaxManaMMC::IntelligenceDef, Spec, Params, Intelligence);
Intelligence = FMath::Max(0.f, Intelligence);
const int32 Level = ResolveLevel(Spec);
const float Base = 50.f;
const float FromInt = 2.5f * Intelligence;
const float FromLevel = 15.f * Level; // example
return Base + FromInt + FromLevel;
```

Notes:
- We set bSnapshot=false in capture definitions so MMCs re-evaluate when captured Attributes change.
- You can optionally scale MMC output via coefficients in the GE modifier (pre/post add/mul).

## Editor wiring: the infinite "Secondary" GE
For your GE that sets derived attributes (MaxHealth/MaxMana):
- Duration Policy: Infinite
- Modifiers:
  - Attribute: Max Health
    - Op: Override
    - Magnitude: Custom Calculation Class = MMC_MaxHealth
  - Attribute: Max Mana
    - Op: Override
    - Magnitude: Custom Calculation Class = MMC_MaxMana

## Recompute on Level change
Level is not an Attribute, so aggregators won't auto-recompute MMCs when Level changes. Options:
- Simple: Reapply the infinite GE on level-up (remove+reapply or apply a fresh spec).
- Alternative: Make Level an Attribute and use AttributeBased or captured dependencies.
- SetByCaller: Pass Level on an instant GE and reapply on change.
Recommendation: Reapply the infinite derived GE on level-up.

## Initialize vitals to full (instant GE)
Initialize Health and Mana after secondary attributes are set:
- Create GE_Aura_VitalAttributes (Instant)
  - Modifier 1: Attribute=Health, Op=Override, Magnitude=AttributeBased(MaxHealth from Target)
  - Modifier 2: Attribute=Mana, Op=Override, Magnitude=AttributeBased(MaxMana from Target)
- Apply order in code (AuraCharacterBase): Primary → Secondary (infinite) → Vital (instant)
```cpp
// After applying DefaultPrimary and DefaultSecondary
ApplyEffectToSelf(DefaultVitalAttributes, 1);
```
This ensures the globes start full and stay consistent with current Max values.

## Testing checklist
- Start: Vigor=9, Intelligence=17, Level=1 ⇒ MaxHealth = 80 + 2.5*9 + 10*1 = 112.5
- Change Level to 2, reapply infinite GE ⇒ MaxHealth increases by 10.
- Increase Vigor/Intelligence via GEs ⇒ Max attributes update automatically (no reapply needed).
- Verify Health/Mana initialize to Max via the instant GE.
- Use ShowDebug AbilitySystem and/or breakpoints in MMCs.

## Teachable moments & pitfalls
- Prefer AttributeBased when possible; reach for MMCs when data lives outside Attributes or logic is complex.
- Keep decoupled: depend on ICombatInterface, not concrete classes.
- Remember to set SourceObject on the effect context before applying, or interface calls will see null.
- Use bSnapshot=false for captured Attributes you want to track live.
- Clamp current values in your AttributeSet when Max values change.
- BlueprintReadOnly should not be on private members; use protected or public as appropriate.

## Appendix — Alternatives & tuning
- Make Level an Attribute for automatic recompute.
- SetByCaller for event-driven inputs (still reapply for infinite effects).
- Move coefficients (2.5, 10, 15) into ScalableFloats/Curves for designer tuning.

See also: [Derived (Secondary) Attributes](./derived-attributes.md)