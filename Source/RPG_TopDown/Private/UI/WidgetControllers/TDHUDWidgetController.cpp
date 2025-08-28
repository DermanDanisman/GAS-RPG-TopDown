// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "UI/WidgetControllers/TDHUDWidgetController.h"

#include "AbilitySystem/Attributes/TDAttributeSet.h"
#include "AbilitySystem/Components/TDAbilitySystemComponent.h"

void UTDHUDWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
}

void UTDHUDWidgetController::BindCallbacksToDependencies()
{
	// Validate and cast the bound AttributeSet to your game's concrete type.
	const UTDAttributeSet* CoreAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);

	// Subscribe to GAS attribute change notifications.
	// Each delegate receives FOnAttributeChangeData with the new value; we forward the float to UI.

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);		
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetStaminaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnStaminaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAttributeSet->GetMaxStaminaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxStaminaChanged.Broadcast(Data.NewValue);
		}
	);

	// MESSAGEWIDGETROWDELEGATE USAGE:
	// Forward GameplayEffect asset tags (e.g., "UI.Message.HealthPotion") to the UI for display.
	//
	// HOW IT WORKS:
	// 1. UGASCoreAbilitySystemComponent broadcasts effect asset tags when GEs are applied
	// 2. We filter for "UI.Message.*" tags and look up corresponding DataTable rows
	// 3. Broadcast the full row data to widgets via MessageWidgetRowDelegate
	//
	// DATATABLE SETUP REQUIREMENTS:
	// - Create a DataTable with FGASCoreUIMessageWidgetRow as the row type
	// - Row keys must match tag FNames (e.g., row key "UI.Message.HealthPotion" for tag "UI.Message.HealthPotion")
	// - Populate MessageTag, MessageText, optional MessageWidget class, and MessageImage
	// - Assign the DataTable to MessageWidgetDataTable property
	//
	// WIDGET BINDING:
	// Widgets should bind to MessageWidgetRowDelegate to receive and display these message notifications.
	Cast<UTDAbilitySystemComponent>(AbilitySystemComponent)->OnEffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				// Filter for UI message tags to avoid processing unrelated effect tags
				const FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("UI.Message"));

				if (Tag.MatchesTag(MessageTag))
				{
					// DataTable lookup: row key should match the tag's FName
					const FGASCoreUIMessageWidgetRow* MessageRow = GetDataTableRowByTag<FGASCoreUIMessageWidgetRow>(MessageWidgetDataTable, Tag);
					if (MessageRow && MessageRow->MessageTag.IsValid())
					{
						// Broadcast the complete row to widgets for display
						MessageWidgetRowDelegate.Broadcast(*MessageRow);
					}
					// else: No matching row found; tag may be misconfigured or DataTable incomplete
				}
				// else: Not a UI.Message tag; ignore (could handle other tag families here)
			}
		}
	);
}
