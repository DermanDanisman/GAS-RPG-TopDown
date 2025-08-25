# Custom Calculations (MMCs) for Max Health and Max Mana

Goal: Compute derived attributes using more than just AttributeBased magnitudes. We'll mix a backing Attribute (Vigor/Intelligence) with a non-Attribute value (Level) that lives on PlayerState (or another combatant), exposed via an interface. We'll implement two Modifier Magnitude Calculations (MMCs): one for Max Health and one for Max Mana.

Use this when AttributeBased is not enough — e.g., you need external data (Level, difficulty, equipment) that isn't an Attribute.

## MMC vs AttributeBased — when to choose which?
- AttributeBased is great for "Attribute X drives Attribute Y" and re-evaluates automatically when the captured Attributes change.
- MMC (UGameplayModMagnitudeCalculation) runs custom C++ to return a float magnitude. Use it when:
  - You must combine Attributes with non-Attribute data (like Level on PlayerState).
  - The formula is branching, table-driven, or otherwise too complex for AttributeBased.

In this guide we:
- Keep Level as a variable (int32) on PlayerState (not an Attribute).
- Expose Level through a `Combat` interface so MMCs remain decoupled from concrete classes.
- Build MMCs for Max Health and Max Mana and wire them into an infinite GE with Override modifiers.

## Design overview
- Level lives on PlayerState (replicated int32).
- Any actor that participates in combat implements `ICombatInterface` with `GetCharacterLevel()`.
- MMCs capture the backing Attributes (Vigor for MaxHealth, Intelligence for MaxMana) and also query `GetCharacterLevel()` from the Source/Target via the effect spec's context.
- The infinite GE uses Override + Calculation Class to set MaxHealth/MaxMana.

## Step 1 — Add Level to PlayerState
Define a replicated level on your PlayerState and optionally a setter you'll use on level-up.

```cpp
// PlayerState header (excerpt)
UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Level, Category="Level")
int32 Level = 1;

UFUNCTION()
void OnRep_Level();

UFUNCTION(BlueprintCallable, Category="Level")
void SetLevel(int32 NewLevel);
```

```cpp
// PlayerState source (excerpt)
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyPlayerState, Level);
}

void AMyPlayerState::SetLevel(int32 NewLevel)
{
    if (HasAuthority())
    {
        Level = FMath::Max(1, NewLevel);
        OnRep_Level();
    }
}

void AMyPlayerState::OnRep_Level()
{
    // See "Recomputation on Level change" below for how we refresh derived values.
}
```

## Step 2 — Define a Combat interface to expose Level
Keep MMCs generic by depending on an interface rather than concrete classes.

```cpp
// CombatInterface.h
#pragma once
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

UINTERFACE(BlueprintType)
class UCombatInterface : public UInterface
{ GENERATED_BODY() };

class ICombatInterface
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat")
    int32 GetCharacterLevel() const;
};
```

Implement `ICombatInterface` on PlayerState, Character, and any enemy types that hold level-like data.

## Step 3 — Implement MMCs for Max Health and Max Mana
Each MMC captures its backing Attribute and queries Level via the interface.

```cpp
// MMC_MaxHealth.h
#pragma once
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

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
#include "MMC_MaxHealth.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "CombatInterface.h"
#include "YourProject/AttributeSets/YourAttributeSet.h" // Replace with your AttributeSet

namespace MaxHealthMMC
{
    // Capture Vigor from the Target (no snapshot so it re-evaluates on Vigor changes)
    static FGameplayEffectAttributeCaptureDefinition VigorDef(
        UYourAttributeSet::GetVigorAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
}

UMMC_MaxHealth::UMMC_MaxHealth()
{
    RelevantAttributesToCapture.Add(MaxHealthMMC::VigorDef);
}

static int32 ResolveLevelFromContext(const FGameplayEffectSpec& Spec)
{
    int32 Level = 1;

    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();
    if (const UObject* SourceObj = Ctx.GetSourceObject())
    {
        if (const ICombatInterface* CI = Cast<ICombatInterface>(SourceObj))
        { Level = ICombatInterface::Execute_GetCharacterLevel(SourceObj); }
    }

    if (Level <= 0)
    {
        if (const AActor* Instigator = Ctx.GetOriginalInstigator())
        {
            if (const ICombatInterface* CI = Cast<ICombatInterface>(Instigator))
            { Level = ICombatInterface::Execute_GetCharacterLevel(const_cast<AActor*>(Instigator)); }
        }
    }

    return FMath::Max(1, Level);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvalParams;
    EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(MaxHealthMMC::VigorDef, Spec, EvalParams, Vigor);
    Vigor = FMath::Max(0.f, Vigor);

    const int32 Level = ResolveLevelFromContext(Spec);

    const float Base = 80.f;        // from our starter mapping
    const float FromVigor = 2.5f * Vigor;
    const float FromLevel = 10.f * static_cast<float>(Level);

    return Base + FromVigor + FromLevel;
}
```

Repeat for Max Mana, capturing Intelligence instead of Vigor and using different coefficients:

```cpp
// MMC_MaxMana.h/.cpp (differences only)
namespace MaxManaMMC
{
    static FGameplayEffectAttributeCaptureDefinition IntelligenceDef(
        UYourAttributeSet::GetIntelligenceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
}

// In constructor
RelevantAttributesToCapture.Add(MaxManaMMC::IntelligenceDef);

// In CalculateBaseMagnitude_Implementation
float Intelligence = 0.f;
GetCapturedAttributeMagnitude(MaxManaMMC::IntelligenceDef, Spec, EvalParams, Intelligence);
Intelligence = FMath::Max(0.f, Intelligence);

const float Base = 50.f;        // starter mapping
const float FromInt = 2.0f * Intelligence;
const float FromLevel = 7.f * static_cast<float>(Level); // example level coefficient
return Base + FromInt + FromLevel;
```

## Step 4 — Configure the Gameplay Effect (Editor)
For your infinite derived-attributes GE (or a dedicated one):
1) Duration Policy: Infinite
2) Add two Modifiers:
   - Attribute: Max Health
     - Modifier Op: Override
     - Magnitude: Calculation Class = `MMC_MaxHealth`
   - Attribute: Max Mana
     - Modifier Op: Override
     - Magnitude: Calculation Class = `MMC_MaxMana`
3) Snapshot: not applicable to MMC input; captured Attributes in the MMC are set to not snapshot so they re-evaluate when those Attributes change.

This keeps MaxHealth/MaxMana current with Vigor/Intelligence. Level is queried on demand in the MMC.

## Recomputation when Level changes
Because Level is not an Attribute, the aggregator won't automatically re-run your MMC when Level changes. Choose one:
- Simple: Reapply the infinite GE on level-up (remove + reapply, or apply a fresh spec). This forces MMCs to recompute with the new Level.
- Alternative: Make Level an Attribute. Then derived values can be AttributeBased or declared as captured dependencies and will re-evaluate automatically.
- Advanced: Feed Level via SetByCaller on an instant GE you reapply on level changes. For an infinite GE, SetByCallers are captured at application time; you still need to reapply to update them.

Recommendation: Reapply the infinite derived GE on level-up. It's straightforward and cheap for a small number of rows.

## Testing
- Start with Vigor = 10, Intelligence = 10, Level = 1.
- Observe Max Health = 80 + 2.5×10 + 10×1 = 115.
- Increase Level to 2, reapply the infinite GE, observe Max Health = 125.
- Bump Vigor via a test GE and confirm Max Health updates automatically without reapplication (because Vigor is captured with snapshot = false).
- Use `ShowDebug AbilitySystem` to watch Attribute values; consider adding an Attributes UI panel later.

## Teachable moments and pitfalls
- MMCs are for logic you can't cleanly express with AttributeBased. Prefer AttributeBased when possible for automatic recompute.
- Keep MMCs decoupled by using interfaces. Avoid hard-coding PlayerState/Character classes.
- If you keep Level off-Attribute, you must decide how to refresh effects when Level changes (reapply vs. periodic vs. making Level an Attribute). Avoid periodic ticking unless absolutely required.
- Ensure captured Attributes in MMCs use `bSnapshot=false` so they re-evaluate on Attribute change.
- When Max attributes change, clamp current values in your AttributeSet to avoid over/underflow.

## Appendix — Alternatives and scaling
- SetByCaller: Pass Level as a SetByCaller value on an instant GE and reapply on change.
- Make Level an Attribute: Simplifies recomputation at the cost of adding an Attribute.
- Use ScalableFloats/Curves: Move coefficients (e.g., 2.5, 2.0, 10) into curve tables keyed by Level or difficulty for tuning.

See also: [Derived (Secondary) Attributes](./derived-attributes.md) for AttributeBased mappings and initialization order.