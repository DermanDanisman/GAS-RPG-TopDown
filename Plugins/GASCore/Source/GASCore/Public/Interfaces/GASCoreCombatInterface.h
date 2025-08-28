// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GASCoreCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UGASCoreCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ICombatInterface
 *
 * Lightweight gameplay interface exposing combat-related queries to decouple systems.
 * Current responsibility:
 * - Provide an Actor's level as a non-Attribute integer for calculations (e.g., MMCs).
 *
 * Design notes:
 * - Using a plain virtual here (not UFUNCTION). That keeps it lightweight and allows
 *   direct C++ calls via native pointers. If you need Blueprint implementability
 *   or network RPC hooks, make it a UFUNCTION(BlueprintNativeEvent).
 */
class GASCORE_API IGASCoreCombatInterface
{
	GENERATED_BODY()

public:

	// Return the actor's level for gameplay calculations (e.g., MaxHealth MMC).
	// Implementation can fetch from PlayerState (players) or the Character itself (AI).
	// Contract: Must be fast and safe to call during effect evaluation (no blocking).
	virtual int32 GetActorLevel();
};
