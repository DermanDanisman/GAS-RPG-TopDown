// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "BlueprintFunctionLibrary/TDAbilitySystemLibrary.h"

// Engine
#include "Kismet/GameplayStatics.h"

// Project
#include "Player/TDPlayerState.h"
#include "UI/HUD/TDHUD.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h" // FGASCoreUIWidgetControllerParams

UTDHUDWidgetController* UTDAbilitySystemLibrary::GetHUDWidgetController(const UObject* WorldContextObject)
{
	// Locate player controller 0 in the current world.
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		// Resolve the project's HUD instance.
		if (ATDHUD* TDHUD = Cast<ATDHUD>(PlayerController->GetHUD()))
		{
			// Resolve the player's state (authority owns ASC/AttributeSet for player-controlled pawns).
			if (ATDPlayerState* PlayerState = PlayerController->GetPlayerState<ATDPlayerState>())
			{
				UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
				UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();

				// Bundle all references into controller params for cleaner passing.
				const FGASCoreUIWidgetControllerParams WidgetControllerParams(
					PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);

				return TDHUD->GetHUDWidgetController(WidgetControllerParams);
			}
		}
	}
	return nullptr;
}

UTDAttributeMenuWidgetController* UTDAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	// Locate player controller 0 in the current world.
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		// Resolve the project's HUD instance.
		if (ATDHUD* TDHUD = Cast<ATDHUD>(PlayerController->GetHUD()))
		{
			// Resolve the player's state and related GAS references.
			if (ATDPlayerState* PlayerState = PlayerController->GetPlayerState<ATDPlayerState>())
			{
				UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
				UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();

				const FGASCoreUIWidgetControllerParams WidgetControllerParams(
					PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);

				return TDHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
			}
		}
	}
	return nullptr;
}