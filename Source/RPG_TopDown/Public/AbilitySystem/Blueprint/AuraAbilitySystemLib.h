// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLib.generated.h"

class UTDHUDWidgetController;
class UTDAttributeMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;
class APlayerController;
class APlayerState;

/**
 * UAuraAbilitySystemLib
 *
 * Blueprint Function Library that exposes pure Blueprint nodes to retrieve 
 * singleton-like widget controllers from any widget context.
 * 
 * Provides two main functions:
 * - Get HUD Widget Controller
 * - Get Attribute Menu Widget Controller
 *
 * Both functions resolve the local player's gameplay systems and build
 * the required controller parameters automatically.
 */
UCLASS()
class RPG_TOPDOWN_API UAuraAbilitySystemLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Get HUD Widget Controller from any widget context.
	 * Resolves local PlayerController and PlayerState, builds widget controller parameters,
	 * and retrieves the cached HUD controller from TDHUD.
	 * 
	 * @param WorldContext - Widget or other world context object
	 * @return The cached HUD widget controller, or nullptr if unavailable
	 */
	UFUNCTION(BlueprintPure, Category = "UI|Widget Controller", 
	          meta = (WorldContext = "WorldContext"))
	static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContext);

	/**
	 * Get Attribute Menu Widget Controller from any widget context.
	 * Resolves local PlayerController and PlayerState, builds widget controller parameters,
	 * and retrieves the cached Attribute Menu controller from TDHUD.
	 * 
	 * @param WorldContext - Widget or other world context object  
	 * @return The cached attribute menu widget controller, or nullptr if unavailable
	 */
	UFUNCTION(BlueprintPure, Category = "UI|Widget Controller",
	          meta = (WorldContext = "WorldContext"))
	static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);

private:

	/**
	 * Helper function to resolve the AbilitySystemComponent for the local player.
	 * Prefers PlayerState that implements AbilitySystemInterface, falls back to Pawn.
	 * 
	 * @param PlayerController - The local player controller
	 * @return Valid ASC pointer or nullptr if not found
	 */
	static UAbilitySystemComponent* ResolveAbilitySystemComponent(APlayerController* PlayerController);

	/**
	 * Helper function to resolve the AttributeSet for the local player.
	 * Queries ASC->GetAttributeSet(UTDAttributeSet::StaticClass()).
	 * 
	 * @param AbilitySystemComponent - Valid ASC to query
	 * @return Valid AttributeSet pointer or nullptr if not found  
	 */
	static UAttributeSet* ResolveAttributeSet(UAbilitySystemComponent* AbilitySystemComponent);
};