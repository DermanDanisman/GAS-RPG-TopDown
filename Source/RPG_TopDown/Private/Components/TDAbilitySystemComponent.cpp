// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Components/TDAbilitySystemComponent.h"


UTDAbilitySystemComponent::UTDAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTDAbilitySystemComponent::BindASCDelegates()
{
	Super::BindASCDelegates();
}

void UTDAbilitySystemComponent::HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	Super::HandleGameplayEffectAppliedToSelf(AbilitySystemComponent, GameplayEffectSpec, ActiveGameplayEffectHandle);
}

