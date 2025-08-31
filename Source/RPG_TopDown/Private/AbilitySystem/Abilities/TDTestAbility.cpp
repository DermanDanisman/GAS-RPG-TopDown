// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "AbilitySystem/Abilities/TDTestAbility.h"
#include "RPG_TopDown/RPG_TopDown.h"

UTDTestAbility::UTDTestAbility()
{
	// Set default values for the test ability
	
	// Allow activation without a specific input
	AbilityInputID = INDEX_NONE;
	
	// Simple ability that can be activated for testing
	// Tags can be set in Blueprint derivatives if needed
}

void UTDTestAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Call parent activation
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Simple test logic - just log that the ability was activated
	UE_LOG(LogRPG_TopDown, Log, TEXT("TDTestAbility activated for actor: %s"), 
		ActorInfo && ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("Unknown"));

	// End the ability immediately for this test
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTDTestAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Log that the ability ended
	UE_LOG(LogRPG_TopDown, Log, TEXT("TDTestAbility ended for actor: %s"), 
		ActorInfo && ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("Unknown"));

	// Call parent end ability
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}