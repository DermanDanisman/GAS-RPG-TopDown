// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Input/TDInputConfig.h"

const UInputAction* UTDInputConfig::FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	return Super::FindAbilityInputActionByTag(InputTag, bLogNotFound);
}