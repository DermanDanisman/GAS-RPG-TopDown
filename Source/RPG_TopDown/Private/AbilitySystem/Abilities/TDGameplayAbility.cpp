// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "AbilitySystem/Abilities/TDGameplayAbility.h"

UTDGameplayAbility::UTDGameplayAbility()
{
	// Set default values for the base ability class
	// These can be overridden in subclasses or Blueprint derivatives
	
	// Default activation policy - can be changed per ability
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Default network replication policy - abilities typically replicate to owner only
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	
	// Set default activation flags - most abilities can be activated locally
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}