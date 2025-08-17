// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Components/CoreAbilitySystemComponent.h"


void UCoreAbilitySystemComponent::BindASCDelegates()
{
	if (OnAppliedToSelfHandle.IsValid())
	{
		return; // already bound
	}

	OnAppliedToSelfHandle = OnGameplayEffectAppliedDelegateToSelf.AddUObject(
		this, &UCoreAbilitySystemComponent::HandleGameplayEffectAppliedToSelf);
}

void UCoreAbilitySystemComponent::HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer GameplayTagContainer;
	GameplayEffectSpec.GetAllAssetTags(GameplayTagContainer);
	
	if (!GameplayTagContainer.IsEmpty())
		OnEffectAssetTags.Broadcast(GameplayTagContainer);
}
