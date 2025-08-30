// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "AbilitySystem/Data/GASCoreAttributeInfoDataAsset.h"

FGASCoreAttributeInformation UGASCoreAttributeInfoDataAsset::FindAttributeInfoByTag(const FGameplayTag& AttributeTag,
	bool bLogNotFound) const
{
	for (const FGASCoreAttributeInformation& AttributeInfoRow : AttributeInformation)
	{
		if (AttributeInfoRow.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return AttributeInfoRow;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find Attribute Info for AttributeTag [%s] on AttributeInfo [%s]."), *AttributeTag.ToString(), *GetNameSafe(this));
	}

	return FGASCoreAttributeInformation();
}