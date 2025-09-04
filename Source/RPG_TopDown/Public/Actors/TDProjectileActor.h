// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Actors/GASCoreProjectile.h"
#include "TDProjectileActor.generated.h"

/**
 * ATDProjectileActor
 *
 * TopDown RPG-specific projectile actor that inherits from AGASCoreProjectile.
 * - Provides game-specific collision handling and projectile behavior.
 * - Integrates with the Gameplay Ability System for damage application and effects.
 * - Supports sphere collision detection for target acquisition and damage dealing.
 *
 * This class extends the base GAS projectile functionality with TopDown RPG-specific
 * features and collision responses tailored for overhead perspective gameplay.
 *
 * Note: Inherits from AGASCoreProjectile (the actual base class in GASCore).
 * If AGASCoreProjectileActor is intended as a separate base class, it should be
 * created first in the GASCore plugin.
 */
UCLASS()
class RPG_TOPDOWN_API ATDProjectileActor : public AGASCoreProjectile
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATDProjectileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Called when the sphere collision component overlaps with another primitive component.
	 * Handles projectile collision with targets, applying damage and effects as configured.
	 * 
	 * Note: This method should be implemented if the projectile has collision components.
	 * The base AGASCoreProjectile class may need to be extended to include collision handling.
	 * 
	 * @param OverlappedComponent The sphere collision component that triggered the overlap
	 * @param OtherActor The actor that was hit by the projectile
	 * @param OtherComp The primitive component of the other actor that was hit
	 * @param OtherBodyIndex Index of the body that was hit (for multi-body components)
	 * @param bFromSweep Whether this overlap was generated from a sweep operation
	 * @param SweepResult Hit result information if this was generated from a sweep
	 */
	UFUNCTION()
	virtual void OnSphereCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};