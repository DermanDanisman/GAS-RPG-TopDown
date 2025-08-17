// Fill out your copyright notice in the Description page of Project Settings.

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
