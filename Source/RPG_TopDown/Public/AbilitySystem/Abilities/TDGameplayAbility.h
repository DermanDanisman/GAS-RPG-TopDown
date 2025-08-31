// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TDGameplayAbility.generated.h"

/**
 * UTDGameplayAbility
 *
 * Base gameplay ability class for the RPG TopDown project.
 * - Provides common functionality and project-specific overrides for all abilities.
 * - Use as base for all concrete abilities in the game.
 * - Inherits from UGameplayAbility and can be extended for specific gameplay mechanics.
 */
UCLASS()
class RPG_TOPDOWN_API UTDGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// Sets default values for this ability
	UTDGameplayAbility();

protected:
	// Add any common project-specific functionality here as needed
};