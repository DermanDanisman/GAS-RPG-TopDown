// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Engine & Module Includes =====
#include "CoreMinimal.h"
#include "GASCore/Public/AbilitySystem/Components/GASCoreAttributeInitComponent.h"

#include "TDDefaultAttributeInitComponent.generated.h"

/**
 * UTDDefaultAttributeInitComponent
 *
 * Game-side initializer that applies:
 * - Default primary attributes (instant GE) via the base component’s behavior.
 * - Default secondary/vital attributes via an infinite GE (recomputes on backing-attribute changes).
 *
 * Usage:
 * - Call InitializeDefaultAttributes right after ASC->InitAbilityActorInfo in your Character/Pawn.
 * - In multiplayer, call on the server and let replication update clients.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_TOPDOWN_API UTDDefaultAttributeInitComponent : public UGASCoreAttributeInitComponent
{
	GENERATED_BODY()

public:
	// ===== Construction =====

	/** Default constructor. Enables ticking (optional if you plan to extend with runtime checks). */
	UTDDefaultAttributeInitComponent();

	// ===== UGASCoreAttributeInitComponent override =====

	/**
	 * InitializeDefaultAttributes
	 * Applies project-defined default attributes to the target ASC:
	 * - Primary attributes (instant once)
	 * - Secondary/Vital attributes (infinite, recalculated as primaries change)
	 */
	virtual void InitializeDefaultAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent) const override;
};