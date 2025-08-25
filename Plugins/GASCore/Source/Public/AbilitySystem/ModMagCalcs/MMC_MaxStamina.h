// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxStamina.generated.h"

/**
 * UMMC_MaxStamina
 *
 * Purpose:
 * - Compute a custom magnitude for MaxHealth as a function of:
 *   (a) a captured Attribute on the Target (Endurance), and
 *   (b) a non-Attribute property (Level) exposed via ICombatInterface on the SourceObject.
 *
 * Where it runs:
 * - GAS evaluates MMCs when the owning GameplayEffect's modifier magnitude is needed
 *   (e.g., when applying/re-applying an infinite GE, or when captured attributes update).
 *
 * Attribute capture notes:
 * - Endurance is captured from the Target. We set bSnapshot=false so the captured value
 *   is evaluated live at calculation time using the current aggregator state.
 *
 * SourceObject notes:
 * - We expect the GameplayEffectContext's SourceObject to implement ICombatInterface.
 *   The call will be null-unsafe here (by design) to catch misuse during development.
 *   Ensure the caller adds ContextHandle.AddSourceObject(SomeActorImplementingInterface).
 *
 * Tags:
 * - We pass aggregated source/target tags to the FAggregatorEvaluateParameters. This allows
 *   tag-conditional modifiers in the attribute aggregation to take effect.
 *
 * Rounding:
 * - Uses RoundHalfToEven to keep Max values integral and avoid .5 artifacts. Change/remove
 *   if you prefer fractional Max values or different rounding semantics.
 *
 * Performance:
 * - Captures are registered once in the constructor (RelevantAttributesToCapture).
 * - Calculation executes on the game thread and should be fast (no blocking or heavy allocations).
 */
UCLASS()
class GASCORE_API UMMC_MaxStamina : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MaxStamina();

	/**
	 * Engine calls this when it needs the base magnitude for a GE modifier using this MMC.
	 * Spec contains captured attributes, tags, context (SourceObject/Instigator), SetByCaller values, and GE level.
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// Definition describing which attribute to capture, from whom (Source/Target), and snapshot behavior.
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
};
