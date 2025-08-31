// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "TDInputConfig.generated.h"

/**
 * FTDInputAction
 *
 * Data structure representing a single input action and its associated gameplay tag.
 * Used to map input actions (Enhanced Input) to GAS input tags for abilities.
 */
USTRUCT(BlueprintType)
struct FTDInputAction
{
	GENERATED_BODY()

	/** The Enhanced Input Action asset used for this input. */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	const class UInputAction* InputAction = nullptr;

	/** The gameplay tag that identifies this input (used for ability mapping). */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * UTDInputConfig
 *
 * Data asset that holds a collection of input actions and their associated input tags.
 * Used to configure and map input actions to ability input tags for the GAS input system.
 */
UCLASS()
class RPG_TOPDOWN_API UTDInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * FindAbilityInputActionByTag
	 *
	 * Looks up the input action associated with the given gameplay tag.
	 * @param InputTag The gameplay tag to search for in the AbilityInputActions array.
	 * @param bLogNotFound If true, log an error if the input tag is not found.
	 * @return Pointer to the matching UInputAction, or nullptr if not found.
	 */
	const UInputAction* FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	/** Array of input actions mapped to ability tags. Used for configuring ability input triggers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TArray<FTDInputAction> AbilityInputActions;
};