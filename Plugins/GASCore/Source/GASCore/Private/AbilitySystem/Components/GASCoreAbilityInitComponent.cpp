// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "AbilitySystem/Components/GASCoreAbilityInitComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Components/GASCoreAbilitySystemComponent.h"


UGASCoreAbilityInitComponent::UGASCoreAbilityInitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGASCoreAbilityInitComponent::AddCharacterAbilities()
{
	// Only authority should grant abilities.
	if (!GetOwner()->HasAuthority()) return;
	
	// Find the ASC for this actor (should exist if GAS is set up properly).
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	UGASCoreAbilitySystemComponent* CoreAbilitySystemComponent = Cast<UGASCoreAbilitySystemComponent>(AbilitySystemComponent);
	if (CoreAbilitySystemComponent)
	{
		CoreAbilitySystemComponent->AddCharacterAbilities(StartupAbilities);
	}
}

