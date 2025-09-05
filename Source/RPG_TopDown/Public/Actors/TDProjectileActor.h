// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Actors/GASCoreSpawnedActorByGameplayAbility.h"
#include "TDProjectileActor.generated.h"

UCLASS()
class RPG_TOPDOWN_API ATDProjectileActor : public AGASCoreSpawnedActorByGameplayAbility
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATDProjectileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
