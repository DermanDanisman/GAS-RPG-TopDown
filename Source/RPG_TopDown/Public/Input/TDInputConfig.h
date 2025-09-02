// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Input/GASCoreAbilityInputConfig.h"
#include "TDInputConfig.generated.h"

/**
 * UTDInputConfig
 *
 * Data asset that holds a collection of input actions and their associated input tags.
 * Used to configure and map input actions to ability input tags for the GAS input system.
 */
UCLASS()
class RPG_TOPDOWN_API UTDInputConfig : public UGASCoreAbilityInputConfig
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
	virtual const UInputAction* FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const override;
};