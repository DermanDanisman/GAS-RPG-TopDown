// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Actors/GASCoreGameplayEffectActor.h"
#include "TDGameplayEffectActor.generated.h"

UCLASS()
class RPG_TOPDOWN_API ATDGameplayEffectActor : public AGASCoreGameplayEffectActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATDGameplayEffectActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
