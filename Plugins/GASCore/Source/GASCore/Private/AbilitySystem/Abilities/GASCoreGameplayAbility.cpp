// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "AbilitySystem/Abilities/GASCoreGameplayAbility.h"

#include "Actors/GASCoreSpawnedActorByGameplayAbility.h"
#include "Interfaces/GASCoreCombatInterface.h"

void UGASCoreGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGASCoreGameplayAbility::SpawnActorFromGameplayAbility()
{
	const FGameplayAbilityActivationInfo GameplayAbilityActivationInfo = GetCurrentActivationInfo();
	const bool bIsServer = HasAuthority(&GameplayAbilityActivationInfo);
	if (!bIsServer) return;

	if (!ensureAlwaysMsgf(SpawnActorClass != nullptr, TEXT("ProjectileActorClass is null on %s"), *GetName())) return;

	IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SpawnLocation = CombatInterface->GetAbilitySpawnLocation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		// TODO: Set the projectile rotation.

		AActor* OwnerActor = GetOwningActorFromActorInfo();
		APawn* InstigatorPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
		
		AGASCoreSpawnedActorByGameplayAbility* Projectile = GetWorld()->SpawnActorDeferred<AGASCoreSpawnedActorByGameplayAbility>(
			SpawnActorClass,
			SpawnTransform,
			OwnerActor,
			InstigatorPawn,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		// TODO: Give the projectile a gameplay effect spec for causing damage.
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}
