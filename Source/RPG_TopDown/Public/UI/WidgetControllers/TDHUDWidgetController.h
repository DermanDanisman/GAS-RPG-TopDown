// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetControllers/CoreHUDWidgetController.h"
#include "TDHUDWidgetController.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_TOPDOWN_API UTDHUDWidgetController : public UCoreHUDWidgetController
{
	GENERATED_BODY()

public:

	/** Broadcasts the initial attribute values (health, mana, etc.) to the UI using delegates. */
	virtual void BroadcastInitialValues() override;

	/** Binds attribute change delegates from GAS (e.g. health/mana change) to handler functions. */
	virtual void BindCallbacksToDependencies() override;
};
