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