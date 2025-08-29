// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"
#include "AbilitySystem/Attributes/TDAttributeSet.h"
#include "AbilitySystemComponent.h"

void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	// Ensure we have valid dependencies
	if (!AttributeSet || !AttributeInfoDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTDAttributeMenuWidgetController::BroadcastInitialValues - Missing dependencies (AttributeSet=%s, AttributeInfoDataAsset=%s)"), 
			*GetNameSafe(AttributeSet), *GetNameSafe(AttributeInfoDataAsset));
		return;
	}

	// Cast to specific AttributeSet implementation
	const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);

	// Iterate through all attribute info entries and broadcast initial values
	const TArray<FTDAttributeInfo>& AttributeInformation = AttributeInfoDataAsset->GetAttributeInformation();
	for (const FTDAttributeInfo& Info : AttributeInformation)
	{
		// Only broadcast if we have a valid attribute getter
		if (Info.AttributeGetter.IsValid())
		{
			BroadcastAttributeInfo(Info.AttributeTag, Info.AttributeGetter);
		}
		else if (ensureMsgf(false, TEXT("Invalid AttributeGetter for tag: %s"), *Info.AttributeTag.ToString()))
		{
			UE_LOG(LogTemp, Warning, TEXT("AttributeGetter is invalid for tag: %s"), *Info.AttributeTag.ToString());
		}
	}
}

void UTDAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();

	// Ensure we have valid dependencies
	if (!AttributeSet || !AbilitySystemComponent || !AttributeInfoDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTDAttributeMenuWidgetController::BindCallbacksToDependencies - Missing dependencies"));
		return;
	}

	// Cast to specific AttributeSet implementation for validation
	const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);

	// Subscribe to ASC value change delegates for every row
	const TArray<FTDAttributeInfo>& AttributeInformation = AttributeInfoDataAsset->GetAttributeInformation();
	for (const FTDAttributeInfo& Info : AttributeInformation)
	{
		// Only bind if we have a valid attribute getter
		if (Info.AttributeGetter.IsValid())
		{
			// Optional: verify the AttributeGetter belongs to the same AttributeSet class
			if (Info.AttributeGetter.GetAttributeSetClass() != UTDAttributeSet::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("AttributeGetter for tag %s belongs to different AttributeSet class: Expected %s, Got %s"), 
					*Info.AttributeTag.ToString(),
					*UTDAttributeSet::StaticClass()->GetName(),
					Info.AttributeGetter.GetAttributeSetClass() ? *Info.AttributeGetter.GetAttributeSetClass()->GetName() : TEXT("NULL"));
			}

			// Bind to attribute value change delegate
			// Capture by value to avoid reference issues with loop variables
			FGameplayTag CapturedTag = Info.AttributeTag;
			FGameplayAttribute CapturedAttribute = Info.AttributeGetter;
			
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Info.AttributeGetter)
				.AddLambda([this, CapturedTag, CapturedAttribute](const FOnAttributeChangeData& Data)
				{
					// Re-broadcast the changed attribute using our helper
					BroadcastAttributeInfo(CapturedTag, CapturedAttribute);
				});
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot bind to invalid AttributeGetter for tag: %s"), *Info.AttributeTag.ToString());
		}
	}
}

void UTDAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	// Ensure AttributeInfoDataAsset is valid
	if (!ensure(AttributeInfoDataAsset))
	{
		UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset is null in BroadcastAttributeInfo"));
		return;
	}

	// Ensure AttributeSet is valid
	if (!ensure(AttributeSet))
	{
		UE_LOG(LogTemp, Error, TEXT("AttributeSet is null in BroadcastAttributeInfo"));
		return;
	}

	// Ensure the attribute is valid
	if (!ensure(Attribute.IsValid()))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Attribute in BroadcastAttributeInfo for tag: %s"), *AttributeTag.ToString());
		return;
	}

	// Find the metadata row via the Data Asset using AttributeTag
	FTDAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
	
	// Ensure the row exists (AttributeTag should not be empty if found)
	if (!AttributeInfo.AttributeTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find AttributeInfo for tag: %s"), *AttributeTag.ToString());
		return;
	}

	// Set AttributeValue using Attribute.GetNumericValue(AttributeSet)
	AttributeInfo.AttributeValue = Attribute.GetNumericValue(AttributeSet);

	// Broadcast via AttributeInfoDelegate
	OnAttributeInfoChanged.Broadcast(AttributeInfo);
}
