// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/TDGameplayAbility.h"
#include "TDTestAbility.generated.h"

/**
 * UTDTestAbility
 *
 * Simple test ability for validating the startup ability system.
 * - Can be granted on character initialization to test the ability granting pipeline.
 * - Provides basic activation behavior for testing purposes.
 */
UCLASS()
class RPG_TOPDOWN_API UTDTestAbility : public UTDGameplayAbility
{
	GENERATED_BODY()

public:
	UTDTestAbility();

protected:
	/** Called when the ability is activated. Override to implement ability logic. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Called when the ability ends. Override to implement cleanup logic. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};