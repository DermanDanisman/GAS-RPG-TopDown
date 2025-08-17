// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "UI/WidgetControllers/CoreWidgetController.h"

#include "Attributes/CoreAttributeSet.h"
#include "Components/CoreAbilitySystemComponent.h"

void UCoreWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& InWidgetControllerParams)
{
	// Assign references from the struct to this controller's members.
	// This method is called right after creation, before the widget is used.
	PlayerController = InWidgetControllerParams.PlayerController;
	PlayerState = InWidgetControllerParams.PlayerState;
	AbilitySystemComponent = InWidgetControllerParams.AbilitySystemComponent;
	AttributeSet = InWidgetControllerParams.AttributeSet;
}

void UCoreWidgetController::BroadcastInitialValues()
{

}

void UCoreWidgetController::BindCallbacksToDependencies()
{
	
}