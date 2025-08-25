// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "AbilitySystem/ModMagCalcs/MMC_MaxStamina.h"

#include "Attributes/CoreAttributeSet.h"
#include "Interfaces/CombatInterface.h"

UMMC_MaxStamina::UMMC_MaxStamina()
{
	// Capture Target.Endurance live (bSnapshot=false), so changes to Endurance propagate to this MMC automatically.
	EnduranceDef.AttributeToCapture = UCoreAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	// Register the capture so the engine knows to gather it into the Spec for evaluation.
	RelevantAttributesToCapture.Add(EnduranceDef);
}

float UMMC_MaxStamina::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	/**
	About this function:
	- It is the C++ body of the BlueprintNativeEvent CalculateBaseMagnitude.
	- The ability system invokes it when applying a GE (or when re-evaluating) whose modifier magnitude type is "Custom Calculation Class."
	- Return value is the base magnitude; the GE may further scale it via FCustomCalculationBasedFloat fields (Coefficient, Pre/Post adds).
	- This executes on the game thread; keep it allocation-free and fast. Do not perform blocking I/O or expensive searches.
	- Recompute triggers:
	  * Captured attributes with bSnapshot=false (like Vigor here) automatically re-evaluate when they change.
	  * Non-attribute inputs (like Level via interface) will NOT auto-recompute. Reapply the infinite GE (or override external dependency delegate) when Level changes.
	*/

	// Provide tag context for aggregation: tag-gated modifiers in the attribute pipeline respect these.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	// 1) Read captured Target.Endurance (post-aggregation, with tag conditions applied).
	float Endurance = 0.f;
	GetCapturedAttributeMagnitude(EnduranceDef, Spec, EvalParams, Endurance);
	Endurance = FMath::Max(Endurance, 0.f); // defensive clamp

	// 2) Read non-attribute dependency: Level from SourceObject via ICombatInterface.
	//    Ensure the caller set Context.AddSourceObject(this) (or a relevant actor) when creating the GE spec.
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	// Note: This assumes the cast succeeds. To harden, add null checks and fallbacks (e.g., to OriginalInstigator) and default Level=1.
	const int32 PlayerLevel = CombatInterface ? CombatInterface->GetActorLevel() : 1;

	// 3) Compute base. Designers can additionally scale this in the GE modifier (coefficient/pre/post multipliers) if desired.
	const float Base = 80.f + 2.5f * Endurance + 10.f * PlayerLevel;

	// 4) Apply rounding policy (optional). Engine accepts floats; we round to avoid .5 artifacts in "Max" displays.
	return FMath::RoundHalfToEven(Base);
}