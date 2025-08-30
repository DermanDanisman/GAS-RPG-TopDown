// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Engine & Module Includes =====
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "TDAbilitySystemLibrary.generated.h"

// ===== Forward Declarations (minimize header coupling) =====
class UTDHUDWidgetController;
class UTDAttributeMenuWidgetController;

/**
 * UTDAbilitySystemLibrary
 *
 * Blueprint-accessible helpers to retrieve initialized Widget Controllers from the HUD.
 * These functions assemble required references and ensure the controller is ready to use.
 */
UCLASS()
class RPG_TOPDOWN_API UTDAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * GetHUDWidgetController
	 * Returns a valid HUD Widget Controller, creating/initializing if needed.
	 * WorldContextObject can be any UObject with access to a World (e.g., a Widget or Actor).
	 */
	UFUNCTION(BlueprintPure, Category="Blueprint Ability System Library|Widget Controller", meta=(WorldContext="WorldContextObject"))
	static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContextObject);

	/**
	 * GetAttributeMenuWidgetController
	 * Returns a valid Attribute Menu Widget Controller, creating/initializing if needed.
	 */
	UFUNCTION(BlueprintPure, Category="Blueprint Ability System Library|Widget Controller", meta=(WorldContext="WorldContextObject"))
	static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
};