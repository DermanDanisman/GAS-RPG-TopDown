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

	// Forward GameplayEffect asset tags (e.g., "UI.Message.HealthPotion") to the UI.
	// Note:
	// - We assume AbilitySystemComponent is actually a UCoreAbilitySystemComponent (subclass)
	// - UCoreAbilitySystemComponent must have called BindASCDelegates() to hook into GE application
	Cast<UCoreAbilitySystemComponent>(AbilitySystemComponent)->OnEffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				// We filter for a "UI.Message" parent tag so only tags intended for UI messages get processed here.
				// Example: "UI.Message.HealthPotion".MatchesTag("UI.Message") returns true.
				const FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("UI.Message"));

				if (Tag.MatchesTag(MessageTag))
				{
					// Look up the corresponding UI message row in the DataTable.
					// This assumes the row key is the full tag FName (e.g., "UI.Message.HealthPotion").
					// Safety: MessageWidgetDataTable can be nullptr and FindRow can return nullptr.
					// Guard checks are recommended in production.
					FUIMessageWidgetRow* MessageRow = GetDataTableRowByTag<FUIMessageWidgetRow>(MessageWidgetDataTable, Tag);
					if (MessageRow && MessageRow->MessageTag.IsValid())
					{
						// Example debug output for quick verification.
						const FString MatchedMsg = FString::Printf(TEXT("GE Tag: %s, Message Tag: %s"), *Tag.ToString(), *MessageRow->MessageTag.ToString());
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, MatchedMsg);

						// Broadcast row to the widget layer (assumes you have a corresponding delegate defined,
						// e.g., DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageRow, const FUIMessageWidgetRow&, Row))
						// and a UPROPERTY(BlueprintAssignable) MessageWidgetRowDelegate on the controller.
						MessageWidgetRowDelegate.Broadcast(*MessageRow);
					}
					// else: No row found or invalid MessageTag; silently ignore.
				}
				// else: Not a UI.Message tag; ignore or handle other tag families as needed.
			}
		}
	);
}