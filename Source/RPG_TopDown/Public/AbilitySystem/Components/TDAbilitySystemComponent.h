// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GASCore/Public/AbilitySystem/Components/GASCoreAbilitySystemComponent.h"
#include "TDAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_TOPDOWN_API UTDAbilitySystemComponent : public UGASCoreAbilitySystemComponent
{
	GENERATED_BODY()

public:
	
	UTDAbilitySystemComponent();

	virtual void BindASCDelegates() override;

protected:

	virtual void ClientHandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) override;
	
};
