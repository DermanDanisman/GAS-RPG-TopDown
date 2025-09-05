// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "AbilitySystem/Abilities/GASCoreProjectileAbility.h"

#include "Actors/GASCoreProjectileActor.h"
#include "Interfaces/GASCoreCombatInterface.h"

void UGASCoreProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bIsServer = HasAuthority(&ActivationInfo);
	if (!bIsServer) return;

	IGASCoreCombatInterface* CombatInterface = Cast<IGASCoreCombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SpawnLocation = CombatInterface->GetAbilitySpawnLocation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		// TODO: Set the projectile rotation.

		AActor* OwnerActor = GetOwningActorFromActorInfo();
		APawn* InstigatorPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
		
		AGASCoreProjectileActor* Projectile = GetWorld()->SpawnActorDeferred<AGASCoreProjectileActor>(
			ProjectileActorClass,
			SpawnTransform,
			OwnerActor,
			InstigatorPawn,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		// TODO: Give the projectile a gameplay effect spec for causing damage.
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}
