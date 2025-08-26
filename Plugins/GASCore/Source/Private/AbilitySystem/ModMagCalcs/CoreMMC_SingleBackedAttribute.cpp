// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

// NOTE: This file contains implementation and should typically be a .cpp, not a .h.
// Consider renaming to "MM_Calculation_Actor.cpp" (or similar) and adding it to your build.
// Also update includes to match the actual header path used in your project layout.

#include "AbilitySystem/ModMagCalcs/CoreMMC_SingleBackedAttribute.h"

#include "Attributes/CoreAttributeSet.h"
#include "Interfaces/CombatInterface.h"

UCoreMMC_SingleBackedAttribute::UCoreMMC_SingleBackedAttribute()
{
	// Intentionally empty.
	// Set CapturedAttributeDef and coefficients (BaseMagnitude/AttributeMultiplier/LevelMultiplier)
	// either in a Blueprint child (recommended for iteration) or in a small C++ child constructor.
}

float UCoreMMC_SingleBackedAttribute::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	/*
		About this function:
		- GAS calls this to compute the base magnitude for a modifier that uses this MMC as its source.
		- The returned value is a base; the owning GameplayEffect can additionally apply PreAdd/Coefficient/PostAdd
		  through FCustomCalculationBasedFloat fields configured on the GE modifier.
		- Execution is on the game thread; please keep it allocation-free and fast.
		- Recompute triggers:
			* Captured attributes with bSnapshot=false auto-recompute on attribute change.
			* Non-attribute inputs (e.g., Level via interface) do NOT auto-recompute. Reapply the infinite GE on level change,
			  or model Level as an attribute if you want live updates.
	*/

	// Tag-aware evaluation params so conditionally gated modifiers (by tags) are respected during capture evaluation.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 1) Captured attribute value (post-aggregation). Zero if not configured or not found.
	float AttributeValue = 0.f;
	if (CapturedAttributeDef.AttributeToCapture.IsValid())
	{
		if (const FGameplayEffectAttributeCaptureSpec* Cap =
				Spec.CapturedRelevantAttributes.FindCaptureSpecByDefinition(CapturedAttributeDef, /*bIncludeModifiers=*/true))
		{
			// AttemptCalculateAttributeMagnitude applies the aggregator pipeline and respects the provided tags.
			Cap->AttemptCalculateAttributeMagnitude(EvaluationParameters, AttributeValue);
		}
	}
	// Defensive clamp: keeps negative values from inverting results if that is undesirable for your use case.
	// If you want debuffs to reduce the result via negative attributes, remove this clamp.
	AttributeValue = FMath::Max(AttributeValue, 0.f);

	// 2) External (non-attribute) dependency: Level from SourceObject via ICombatInterface.
	//    Best practice: when creating the GE spec, the applier sets Context.AddSourceObject(SomeActorImplementingICombatInterface).
	//    If absent, we default to 1 here. Alternatively (and more robustly), you can fall back to Spec.GetLevel().
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = CombatInterface ? CombatInterface->GetActorLevel() : 1;

	// 3) Compute the base value. Designers can further scale this in the GE modifier if desired.
	const float FinalValue = BaseMagnitude
		+ AttributeMultiplier * AttributeValue
		+ LevelMultiplier     * PlayerLevel;

	// 4) Apply rounding policy (display-oriented). Engine stores floats; rounding avoids ".5" artifacts in Max displays.
	return FinalizeOutput(FinalValue);
}

const TArray<FGameplayEffectAttributeCaptureDefinition>& UCoreMMC_SingleBackedAttribute::GetAttributeCaptureDefinitions() const
{
	// Ensure the engine captures whatever attribute is configured on the CDO.
	// Returning this list here (instead of pushing into RelevantAttributesToCapture) guarantees that the
	// capture set is available when the GameplayEffectSpec is constructed.
	CachedCaptures.Reset();
	if (CapturedAttributeDef.AttributeToCapture.IsValid())
	{
		CachedCaptures.Add(CapturedAttributeDef);
	}
	return CachedCaptures;
}

float UCoreMMC_SingleBackedAttribute::FinalizeOutput(float Value) const
{
	switch (RoundingPolicy)
	{
	case EMMCRoundingPolicy::RoundHalfToEven:
		return FMath::RoundHalfToEven(Value);
	case EMMCRoundingPolicy::Floor:
		return FMath::FloorToFloat(Value);
	case EMMCRoundingPolicy::Ceil:
		return FMath::CeilToFloat(Value);
	default:
		return Value; // EMMCRoundingPolicy::None
	}
}