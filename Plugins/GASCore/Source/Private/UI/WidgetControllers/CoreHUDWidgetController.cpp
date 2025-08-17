// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetControllers/CoreHUDWidgetController.h"

#include "Attributes/CoreAttributeSet.h"
#include "Components/CoreAbilitySystemComponent.h"

void UCoreHUDWidgetController::BroadcastInitialValues()
{
	const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);
	
	// Broadcast the current values of health, max health, mana, and max mana to the UI.
	OnHealthChanged.Broadcast(CoreAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(CoreAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(CoreAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(CoreAttributeSet->GetMaxMana());
}

void UCoreHUDWidgetController::BindCallbacksToDependencies()
{
	const UCoreAttributeSet* CoreAttributeSet = CastChecked<UCoreAttributeSet>(AttributeSet);

	// Bind the GAS attribute change delegates to this controller's methods for real-time updates.
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

	Cast<UCoreAbilitySystemComponent>(AbilitySystemComponent)->OnEffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("UI.Message"));
				// "UI.Message.HealthPotion".MatchesTag("UI.Message") will return True,
				// "UI.Message".MatchesTag("UI.Message.HealthPotion") will return False
				if (Tag.MatchesTag(MessageTag))
				{
					FUIMessageWidgetRow* MessageRow = GetDataTableRowByTag<FUIMessageWidgetRow>(MessageWidgetDataTable, Tag);
					if (MessageRow->MessageTag.IsValid())
					{
						const FString MatchedMsg = FString::Printf(TEXT("GE Tag: %s, Message Tag: %s"), *Tag.ToString(), *MessageRow->MessageTag.ToString());
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, MatchedMsg);
						MessageWidgetRowDelegate.Broadcast(*MessageRow);
					}
				}
			}
		}
	);
}