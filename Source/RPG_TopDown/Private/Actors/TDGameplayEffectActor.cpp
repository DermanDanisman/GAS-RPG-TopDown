// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Actors/TDGameplayEffectActor.h"


// Sets default values
ATDGameplayEffectActor::ATDGameplayEffectActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATDGameplayEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATDGameplayEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

