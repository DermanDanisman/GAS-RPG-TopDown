// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "AbilitySystem/Components/TDAbilitySystemComponent.h"

#include "TDGameplayTags.h"


UTDAbilitySystemComponent::UTDAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTDAbilitySystemComponent::BindASCDelegates()
{
	Super::BindASCDelegates();

	const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();
	/*GEngine->AddOnScreenDebugMessage(-1,
		10.f,
		FColor::Orange,
		FString::Printf(TEXT("Tag: %s"), *GameplayTags.Attributes_Secondary_Armor.ToString()));*/
}

void UTDAbilitySystemComponent::HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	Super::HandleGameplayEffectAppliedToSelf(AbilitySystemComponent, GameplayEffectSpec, ActiveGameplayEffectHandle);
}

