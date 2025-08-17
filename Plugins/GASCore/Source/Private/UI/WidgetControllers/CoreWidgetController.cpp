// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Implementation notes:
// - Base class only wires references and defines no-op hooks for initialization/binding.
// - Derived controllers (e.g., UCoreHUDWidgetController) should override BroadcastInitialValues()
//   and BindCallbacksToDependencies() to push the current state and wire reactive updates.
// - Call order from HUD is typically:
//     1) Create controller
//     2) SetWidgetControllerParams(params)
//     3) Widget.SetWidgetController(Controller) -> WidgetControllerSet (BP event binds UI)
//     4) Controller.BindCallbacksToDependencies()
//     5) Controller.BroadcastInitialValues()

#include "UI/WidgetControllers/CoreWidgetController.h"

void UCoreWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& InWidgetControllerParams)
{
	// Assign references from the struct to this controller's members.
	// This should be called before any binding or broadcasting occurs.
	PlayerController       = InWidgetControllerParams.PlayerController;
	PlayerState            = InWidgetControllerParams.PlayerState;
	AbilitySystemComponent = InWidgetControllerParams.AbilitySystemComponent;
	AttributeSet           = InWidgetControllerParams.AttributeSet;

	// Optional: add defensive checks in derived classes (e.g., ensure non-null ASC/AttributeSet).
	// check(AbilitySystemComponent);
	// check(AttributeSet);
}

void UCoreWidgetController::BroadcastInitialValues()
{
	// Base class: intentionally empty.
	// Derived controllers should:
	// - Read current values from the AttributeSet (e.g., GetHealth/GetMaxHealth)
	// - Broadcast via BlueprintAssignable delegates so widgets receive initial state
	//
	// Example (in derived):
	//   OnHealthChanged.Broadcast(CoreAS->GetHealth());
	//   OnMaxHealthChanged.Broadcast(CoreAS->GetMaxHealth());
}

void UCoreWidgetController::BindCallbacksToDependencies()
{
	// Base class: intentionally empty.
	// Derived controllers should:
	// - Subscribe to ASC/AttributeSet delegates for value changes
	// - Forward changes via controller delegates for the widgets to consume
	//
	// Example (in derived):
	//   AbilitySystemComponent
	//     ->GetGameplayAttributeValueChangeDelegate(CoreAS->GetHealthAttribute())
	//     .AddLambda([this](const FOnAttributeChangeData& Data)
	//     {
	//         OnHealthChanged.Broadcast(Data.NewValue);
	//     });
}