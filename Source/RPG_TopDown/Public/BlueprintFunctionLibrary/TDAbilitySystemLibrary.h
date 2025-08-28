// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/HUD/TDHUD.h"
#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"
#include "TDAbilitySystemLibrary.generated.h"

class UTDHUDWidgetController;
/**
 * 
 */
UCLASS()
class RPG_TOPDOWN_API UTDAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Blueprint Ability System Library|Widget Controller")
	static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Blueprint Ability System Library|Widget Controller")
	static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
};
