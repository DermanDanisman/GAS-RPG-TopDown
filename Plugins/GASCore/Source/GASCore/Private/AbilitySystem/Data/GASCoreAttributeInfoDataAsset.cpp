// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

// Implementation for UGASCoreAttributeInfoDataAsset::FindAttributeInfoByTag

#include "AbilitySystem/Data/GASCoreAttributeInfoDataAsset.h"

FGASCoreAttributeInformation UGASCoreAttributeInfoDataAsset::FindAttributeInfoByTag(
	const FGameplayTag& AttributeTag,
	bool bLogNotFound) const
{
	// Linear search across authored rows; acceptable for small lists typically used for UI display.
	for (const FGASCoreAttributeInformation& AttributeInfoRow : AttributeInformation)
	{
		// Exact match is intended: UI rows are defined at a specific tag granularity.
		if (AttributeInfoRow.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return AttributeInfoRow; // Return by value (BP-friendly)
		}
	}

	// Optionally report missing tags to help diagnose misconfigured assets.
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Can't find Attribute Info for AttributeTag [%s] on AttributeInfo [%s]."),
			*AttributeTag.ToString(), *GetNameSafe(this));
	}

	// Not found: return a default-constructed row (AttributeValue remains 0.f).
	return FGASCoreAttributeInformation();
}