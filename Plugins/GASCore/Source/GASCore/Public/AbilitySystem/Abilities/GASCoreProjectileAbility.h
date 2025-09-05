// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GASCoreGameplayAbility.h"
#include "GASCoreProjectileAbility.generated.h"

class AGASCoreSpawnedActorByGameplayAbility;
/**
 * 
 */
UCLASS()
class GASCORE_API UGASCoreProjectileAbility : public UGASCoreGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void SpawnActorFromGameplayAbility() override;


};
