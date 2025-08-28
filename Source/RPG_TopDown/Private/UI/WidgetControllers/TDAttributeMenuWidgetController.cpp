// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"

void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
	// Call parent implementation
	Super::BroadcastInitialValues();
	
	// Minimal implementation - projects can extend this to broadcast
	// initial attribute values to the UI when the controller is first set up
}

void UTDAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	// Call parent implementation  
	Super::BindCallbacksToDependencies();
	
	// Minimal implementation - projects can extend this to subscribe
	// to attribute change delegates on the AbilitySystemComponent
}