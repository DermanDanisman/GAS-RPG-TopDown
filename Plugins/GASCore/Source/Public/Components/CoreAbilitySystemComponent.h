// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CoreAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTagsSignature, const FGameplayTagContainer& /*AssetTags*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASCORE_API UCoreAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	void BindASCDelegates();

	FEffectAssetTagsSignature OnEffectAssetTags;

protected:

	void HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);

private:

	// Prevent double-binding and allow unbinding
	FDelegateHandle OnAppliedToSelfHandle;
};
