// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GASCoreAbilityInputConfig.generated.h"

/**
 * FGASCoreInputAction
 *
 * Data structure representing a single input action and its associated gameplay tag.
 * Used to map Enhanced Input actions to gameplay tags for ability routing.
 */
USTRUCT(BlueprintType)
struct GASCORE_API FGASCoreAbilityInputAction
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
 * UGASCoreInputConfig
 *
 * Data asset that holds a collection of input actions and their associated input tags.
 * Used to configure and map input actions to ability input tags for the GAS input system.
 * Place instances of this Data Asset in your game content and reference it from your PlayerController.
 */
UCLASS()
class GASCORE_API UGASCoreAbilityInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * Looks up the input action associated with the given gameplay tag.
	 * @param InputTag The gameplay tag to search for in the AbilityInputActions array.
	 * @param bLogNotFound If true, log an error if the input tag is not found.
	 * @return Pointer to the matching UInputAction, or nullptr if not found.
	 */
	virtual const UInputAction* FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	/** Array of input actions mapped to ability tags. Used for configuring ability input triggers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GASCore|Ability Input Config|Input")
	TArray<FGASCoreAbilityInputAction> AbilityInputActions;
};