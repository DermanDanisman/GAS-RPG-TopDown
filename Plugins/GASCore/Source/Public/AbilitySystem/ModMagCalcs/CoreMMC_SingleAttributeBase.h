// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"

#include "CoreMMC_SingleAttributeBase.generated.h"

UENUM(BlueprintType)
enum class EMMCRoundingPolicy : uint8
{
	None,
	RoundHalfToEven,
	Floor,
	Ceil
};

/**
 * UCoreMMC_SingleAttributeBase
 *
 * Minimal, single-backed-attribute Mod Magnitude Calculation (MMC).
 *
 * Formula:
 *   Final = BaseMagnitude + AttributeMultiplier * CapturedAttribute + LevelMultiplier * Level
 *
 * Where:
 * - CapturedAttribute is read via the capture defined in CapturedAttributeDef
 *   (set AttributeToCapture, AttributeSource, and bSnapshot in BP or C++).
 * - Level is resolved from ICombatInterface on the Spec's Context.SourceObject if available,
 *   otherwise (see .cpp) you may choose to fall back to Spec.GetLevel().
 *
 * Notes:
 * - This class overrides GetAttributeCaptureDefinitions() so that GAS will capture the attribute
 *   you configured on the class default object (CDO) before any GameplayEffectSpec uses it.
 * - CalculateBaseMagnitude_Implementation returns the "base magnitude." The owning GameplayEffect
 *   can still apply its own PreAdd/Coefficient/PostAdd via FCustomCalculationBasedFloat.
 * - Keep this calculation allocation-free and fast; it runs on the game thread.
 */
UCLASS()
class GASCORE_API UCoreMMC_SingleAttributeBase : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UCoreMMC_SingleAttributeBase();

	// Called by GAS to compute the base magnitude for a GE modifier that references this MMC.
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

	// Returns the capture definitions to register on the Spec; ensures our attribute is captured.
	virtual const TArray<FGameplayEffectAttributeCaptureDefinition>& GetAttributeCaptureDefinitions() const override;

protected:
	// Single attribute to capture (choose Attribute, Source, and Snapshot in BP).
	// Example (C++):
	//   CaptureDef.AttributeToCapture = UCoreAttributeSet::GetVigorAttribute();
	//   CaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	//   CaptureDef.bSnapshot = false; // live updates when attribute changes
	UPROPERTY(EditAnywhere, Category="MMC|Properties", meta=(ToolTip="Attribute capture definition (Attribute/Source/Snapshot). Must be set on the class default object so GAS can capture it."))
	FGameplayEffectAttributeCaptureDefinition CapturedAttributeDef;

	// Constant base part of the formula.
	UPROPERTY(EditAnywhere, Category="MMC|Properties")
	float BaseMagnitude = 100.f;

	// Multiplies the Actor Level (see .cpp).
	UPROPERTY(EditAnywhere, Category="MMC|Properties", meta=(ToolTip="Coefficient applied to Level. Consider reapplying the GE on level change, or model Level as an attribute for live recompute."))
	float LevelMultiplier = 10.f;

	// Multiplies the captured attribute value.
	UPROPERTY(EditAnywhere, Category="MMC|Properties")
	float AttributeMultiplier = 1.f;

	// Optional rounding of the final output (display convenience).
	UPROPERTY(EditAnywhere, Category="MMC|Rounding Policy")
	EMMCRoundingPolicy RoundingPolicy = EMMCRoundingPolicy::RoundHalfToEven;

private:
	// Backing array returned by GetAttributeCaptureDefinitions(); must be a stable ref.
	mutable TArray<FGameplayEffectAttributeCaptureDefinition> CachedCaptures;

	// Applies the rounding policy. (No clamping here to keep logic minimal.)
	float FinalizeOutput(float Value) const;
};