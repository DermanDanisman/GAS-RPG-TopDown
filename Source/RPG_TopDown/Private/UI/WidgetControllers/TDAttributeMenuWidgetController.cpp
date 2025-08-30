// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

// Implementation (".cpp") for UTDAttributeMenuWidgetController.
// This file demonstrates the generic, data-driven broadcast using the AttributeSet registry.

#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"

#include "AbilitySystem/Attributes/TDAttributeSet.h"
#include "AbilitySystem/Data/TDAttributeInfo.h"

void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
	// The controller must have a valid data asset to provide UI metadata (names/icons/etc.).
	check(AttributeInfoDataAsset);
	
	for (const auto& AttributeInfoRow : AttributeInfoDataAsset->GetAttributeInformation())
	{
		if (!AttributeInfoRow.AttributeGetter.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("AttributeGetter not set for row '%s' in '%s'"),
				*AttributeInfoRow.AttributeName.ToString(), *GetNameSafe(AttributeInfoDataAsset));
			continue;
		}
		BroadcastAttributeInfo(AttributeInfoRow.AttributeTag);
	}
}

void UTDAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	check(AttributeInfoDataAsset);

	for (const auto& AttributeInfoRow : AttributeInfoDataAsset->GetAttributeInformation())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeInfoRow.AttributeGetter).AddLambda(
			[this, AttributeInfoRow](const FOnAttributeChangeData& AttributeChangedData)
			{
				BroadcastAttributeInfo(AttributeInfoRow.AttributeTag);
			}
		);
	}
}

void UTDAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag) const
{
	FGASCoreAttributeInformation Info = AttributeInfoDataAsset->FindAttributeInfoByTag(AttributeTag);
	Info.AttributeValue = Info.AttributeGetter.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
