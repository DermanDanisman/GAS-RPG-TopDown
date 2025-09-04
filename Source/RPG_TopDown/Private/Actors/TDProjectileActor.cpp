// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Actors/TDProjectileActor.h"

// Sets default values
ATDProjectileActor::ATDProjectileActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ATDProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATDProjectileActor::OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Implementation for collision handling
	// This method provides collision handling for the TopDown RPG projectile
	// Add your collision logic here (damage application, effects, etc.)
	
	// Example: Basic collision response
	if (OtherActor && OtherActor != GetOwner())
	{
		// Handle projectile impact with target
		// Apply damage or effects through the Gameplay Ability System
		// Destroy or modify projectile behavior as needed
	}
}