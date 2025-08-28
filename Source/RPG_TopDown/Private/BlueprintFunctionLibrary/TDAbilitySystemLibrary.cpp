// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "BlueprintFunctionLibrary/TDAbilitySystemLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Player/TDPlayerState.h"
#include "UI/HUD/TDHUD.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h"

UTDHUDWidgetController* UTDAbilitySystemLibrary::GetHUDWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ATDHUD* TDHUD = Cast<ATDHUD>(PlayerController->GetHUD()))
		{
			if (ATDPlayerState* PlayerState = PlayerController->GetPlayerState<ATDPlayerState>())
			{
				UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
				UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();
				const FGASCoreUIWidgetControllerParams WidgetControllerParams(PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
				return TDHUD->GetHUDWidgetController(WidgetControllerParams);
			}
		}
	}
	return nullptr;
}

UTDAttributeMenuWidgetController* UTDAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ATDHUD* TDHUD = Cast<ATDHUD>(PlayerController->GetHUD()))
		{
			if (ATDPlayerState* PlayerState = PlayerController->GetPlayerState<ATDPlayerState>())
			{
				UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
				UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();
				const FGASCoreUIWidgetControllerParams WidgetControllerParams(PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
				return TDHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
			}
		}
	}
	return nullptr;
}
