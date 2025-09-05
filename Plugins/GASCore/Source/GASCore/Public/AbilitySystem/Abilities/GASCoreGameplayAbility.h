// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GASCoreGameplayAbility.generated.h"

class AGASCoreSpawnedActorByGameplayAbility;
/**
 * 
 */
UCLASS()
class GASCORE_API UGASCoreGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category="GASCore|Gameplay Ability|Tag")
	FGameplayTag StartupInputTag;

	UFUNCTION(BlueprintPure, Category = "GASCore|Projectile Ability")
	TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> GetSpawnActorClass() { return SpawnActorClass; }

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category="GASCore|Gameplay Ability")
	virtual void SpawnActorFromGameplayAbility();

	UPROPERTY(EditAnywhere, Category = "GASCore|Projectile Ability|Spawn Actor")
	TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> SpawnActorClass;
};
