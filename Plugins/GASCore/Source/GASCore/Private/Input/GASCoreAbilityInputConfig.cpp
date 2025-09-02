// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Input/GASCoreAbilityInputConfig.h"
#include "InputAction.h"

const UInputAction* UGASCoreAbilityInputConfig::FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	// Validate the input tag.
	if (!InputTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("InputTag parameter is invalid in InputConfig [%s]"), *GetNameSafe(this));
		return nullptr;
	}
	// Check if the array of ability input actions is empty.
	if (AbilityInputActions.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("AbilityInputActions array is empty in InputConfig [%s]"), *GetNameSafe(this));
		return nullptr;
	}
	
	// Search for a matching input action by tag.
	for (const FGASCoreAbilityInputAction& AbilityInputAction : AbilityInputActions)
	{
		// Use MatchesTag to allow for parent/child tag relationships.
		if (AbilityInputAction.InputTag.MatchesTag(InputTag))
		{
			return AbilityInputAction.InputAction;
		}
	}

	// If not found, optionally log an error.
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find Ability Input Action for InputTag [%s] in InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}
	return nullptr;
}