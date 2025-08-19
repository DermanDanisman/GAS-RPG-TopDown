// Copyright:
// © 2025 Heathrow (Derman). All rights reserved.
//
// Implementation notes:
// - This controller assumes AttributeSet is of type UCoreAttributeSet (cast-checked)
// - Binds to ASC delegates using AddLambda with a capture of `this`
//   Ensure this controller's lifetime outlives the ASC bindings, or use weak captures/handles
// - For Effect Asset Tags, we rely on UCoreAbilitySystemComponent to broadcast OnEffectAssetTags
// - UI message routing expects a DataTable with rows keyed by tag FNames (Tag.GetTagName())

#include "UI/WidgetControllers/CoreHUDWidgetController.h"

#include "Attributes/CoreAttributeSet.h"
#include "Components/CoreAbilitySystemComponent.h"

void UCoreHUDWidgetController::BroadcastInitialValues()
{
	// Validate and cast the bound AttributeSet to your game's concrete type.
	const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);
	
	// Push the current attribute values to any bound widgets so they can initialize their displays.
	// Order does not matter; each delegate is independent.
	OnHealthChanged.Broadcast(CoreAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(CoreAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(CoreAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(CoreAttributeSet->GetMaxMana());
	OnStaminaChanged.Broadcast(CoreAttributeSet->GetStamina());
	OnMaxStaminaChanged.Broadcast(CoreAttributeSet->GetMaxStamina());
}

void UCoreHUDWidgetController::BindCallbacksToDependencies()
{
	// Validate and cast the bound AttributeSet to your game's concrete type.
	const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);

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
	// 1. UCoreAbilitySystemComponent broadcasts effect asset tags when GEs are applied
	// 2. We filter for "UI.Message.*" tags and look up corresponding DataTable rows
	// 3. Broadcast the full row data to widgets via MessageWidgetRowDelegate
	//
	// DATATABLE SETUP REQUIREMENTS:
	// - Create a DataTable with FUIMessageWidgetRow as the row type
	// - Row keys must match tag FNames (e.g., row key "UI.Message.HealthPotion" for tag "UI.Message.HealthPotion")
	// - Populate MessageTag, MessageText, optional MessageWidget class, and MessageImage
	// - Assign the DataTable to MessageWidgetDataTable property
	//
	// WIDGET BINDING:
	// Widgets should bind to MessageWidgetRowDelegate to receive and display these message notifications.
	Cast<UCoreAbilitySystemComponent>(AbilitySystemComponent)->OnEffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				// Filter for UI message tags to avoid processing unrelated effect tags
				const FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("UI.Message"));

				if (Tag.MatchesTag(MessageTag))
				{
					// DataTable lookup: row key should match the tag's FName
					FUIMessageWidgetRow* MessageRow = GetDataTableRowByTag<FUIMessageWidgetRow>(MessageWidgetDataTable, Tag);
					if (MessageRow && MessageRow->MessageTag.IsValid())
					{
						// DEBUG: Optional on-screen verification (should be disabled in shipping builds)
#if !UE_BUILD_SHIPPING
						const FString MatchedMsg = FString::Printf(TEXT("GE Tag: %s, Message Tag: %s"), *Tag.ToString(), *MessageRow->MessageTag.ToString());
						UE_LOG(LogTemp, Verbose, TEXT("%s"), *MatchedMsg);
#endif

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