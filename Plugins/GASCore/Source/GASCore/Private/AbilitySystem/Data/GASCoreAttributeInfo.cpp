// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "AbilitySystem/Data/GASCoreAttributeInfo.h"
#include "Utilities/GASCoreLogging.h"

// Define the static default instance
const FGASCoreAttributeInfo UGASCoreAttributeInfo::DefaultAttributeInfo = FGASCoreAttributeInfo();

const FGASCoreAttributeInfo& UGASCoreAttributeInfo::FindAttributeInfoByTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	// Iterate through all attribute entries
	for (const FGASCoreAttributeInfo& Info : AttributeInformation)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}
	
	// Handle not found case
	if (bLogNotFound)
	{
		UE_LOG(LogGASCore, Warning, TEXT("AttributeInfo not found for tag: %s in data asset: %s"), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	
	// Return reference to default static instance if not found
	return DefaultAttributeInfo;
}

TArray<FGASCoreAttributeInfo> UGASCoreAttributeInfo::GetPrimaryAttributes() const
{
	TArray<FGASCoreAttributeInfo> PrimaryAttributes;
	for (const FGASCoreAttributeInfo& Info : AttributeInformation)
	{
		if (Info.bIsPrimary)
		{
			PrimaryAttributes.Add(Info);
		}
	}
	return PrimaryAttributes;
}

TArray<FGASCoreAttributeInfo> UGASCoreAttributeInfo::GetSecondaryAttributes() const
{
	TArray<FGASCoreAttributeInfo> SecondaryAttributes;
	for (const FGASCoreAttributeInfo& Info : AttributeInformation)
	{
		if (!Info.bIsPrimary)
		{
			SecondaryAttributes.Add(Info);
		}
	}
	return SecondaryAttributes;
}