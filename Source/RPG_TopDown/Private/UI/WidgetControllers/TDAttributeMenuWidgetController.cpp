// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

// Implementation (".cpp") for UTDAttributeMenuWidgetController.
// This file demonstrates the generic, data-driven broadcast using the AttributeInfo Data Asset rows.

#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"

#include "AbilitySystem/Attributes/TDAttributeSet.h"   // For AttributeSet type used by GetNumericValue
#include "AbilitySystem/Data/TDAttributeInfo.h"        // For AttributeInfo data asset definitions

void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
	// The controller must have a valid Data Asset to provide UI metadata and attribute identities.
	check(AttributeInfoDataAsset);

	// Iterate all authored UI rows (Tag + Name/Description + AttributeGetter).
	for (const FGASCoreAttributeInformation& AttributeInfoRow : AttributeInfoDataAsset->GetAttributeInformation())
	{
		// Defensive: Author each row with its FGameplayAttribute identity.
		if (!AttributeInfoRow.AttributeGetter.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("AttributeGetter not set for row '%s' in '%s'"),
				*AttributeInfoRow.AttributeName.ToString(), *GetNameSafe(AttributeInfoDataAsset));
			continue;
		}

		// Compute and broadcast this row's current value to any UI listeners.
		BroadcastAttributeInfo(AttributeInfoRow.AttributeTag);
	}
}

void UTDAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	// Data Asset must be set; ASC/AttributeSet validity is ensured by the base controller's lifecycle.
	check(AttributeInfoDataAsset);

	// Bind a value-change callback per authored row (one per FGameplayAttribute identity).
	for (const FGASCoreAttributeInformation& AttributeInfoRow : AttributeInfoDataAsset->GetAttributeInformation())
	{
		// Note: If a row has no AttributeGetter, there's nothing to bind for live updates.
		if (!AttributeInfoRow.AttributeGetter.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping delegate bind; AttributeGetter not set for row '%s' in '%s'"),
				*AttributeInfoRow.AttributeName.ToString(), *GetNameSafe(AttributeInfoDataAsset));
			continue;
		}

		// Subscribe to ASC's per-attribute change delegate.
		// Capture the row by value on purpose (loop locals go out of scope after binding).
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(AttributeInfoRow.AttributeGetter)
			.AddLambda([this, AttributeInfoRow](const FOnAttributeChangeData& /*AttributeChangedData*/)
			{
				// On change, recompute and broadcast only this row.
				BroadcastAttributeInfo(AttributeInfoRow.AttributeTag);
			});
	}
}

void UTDAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag) const
{
	// Lookup authored metadata (Name/Description/Getter) by tag.
	FGASCoreAttributeInformation Info = AttributeInfoDataAsset->FindAttributeInfoByTag(AttributeTag);

	// Compute the current numeric value from our AttributeSet using the row's FGameplayAttribute identity.
	// Note: GetNumericValue accepts the base UAttributeSet*; a concrete cast is not strictly required here.
	Info.AttributeValue = Info.AttributeGetter.GetNumericValue(AttributeSet);

	// Notify any bound UI widgets. Widgets can filter by matching Info.AttributeTag in BP.
	AttributeInfoDelegate.Broadcast(Info);
}