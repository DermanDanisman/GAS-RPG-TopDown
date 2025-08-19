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
	// BASE CLASS IMPLEMENTATION: Intentionally empty (no-op).
	//
	// DERIVED CONTROLLER RESPONSIBILITIES:
	// Derived controllers (e.g., UCoreHUDWidgetController) should override this method to:
	// 1. Read current values from the AttributeSet (e.g., GetHealth(), GetMaxHealth(), GetMana())
	// 2. Broadcast these values via BlueprintAssignable delegates so widgets receive initial state
	// 3. Ensure UI displays correct values immediately after controller/widget setup
	//
	// INITIALIZATION ORDER:
	// This should be called AFTER:
	// - SetWidgetControllerParams() (establishes valid ASC/AttributeSet references)
	// - Widget.SetWidgetController() (widget is ready to receive broadcasts)
	// - BindCallbacksToDependencies() (ongoing updates are subscribed)
	//
	// EXAMPLE IMPLEMENTATION (in derived class):
	//   const UCoreAttributeSet* CoreAS = CastChecked<UCoreAttributeSet>(AttributeSet);
	//   OnHealthChanged.Broadcast(CoreAS->GetHealth());
	//   OnMaxHealthChanged.Broadcast(CoreAS->GetMaxHealth());
	//   OnManaChanged.Broadcast(CoreAS->GetMana());
	//   // ... etc for all attributes that have corresponding UI elements
}

void UCoreWidgetController::BindCallbacksToDependencies()
{
	// BASE CLASS IMPLEMENTATION: Intentionally empty (no-op).
	//
	// DERIVED CONTROLLER RESPONSIBILITIES:
	// Derived controllers should override this method to:
	// 1. Subscribe to ASC/AttributeSet delegates for value changes
	// 2. Forward those changes via controller delegates for widgets to consume
	// 3. Subscribe to custom ASC delegates (e.g., effect application events, ability cooldowns)
	//
	// BINDING PATTERN:
	// - Use AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate() for attribute changes
	// - Use AddLambda, AddUObject, or AddDynamic depending on lifetime management needs
	// - Forward received data through controller's BlueprintAssignable delegates
	//
	// INITIALIZATION ORDER:
	// This should be called AFTER:
	// - SetWidgetControllerParams() (establishes valid ASC/AttributeSet references)
	// - Widget.SetWidgetController() (widget delegates are ready to be bound to)
	// But BEFORE:
	// - BroadcastInitialValues() (ensures ongoing updates are wired before initial state push)
	//
	// EXAMPLE IMPLEMENTATION (in derived class):
	//   const UCoreAttributeSet* CoreAS = CastChecked<UCoreAttributeSet>(AttributeSet);
	//   
	//   AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CoreAS->GetHealthAttribute())
	//     .AddLambda([this](const FOnAttributeChangeData& Data)
	//     {
	//         OnHealthChanged.Broadcast(Data.NewValue);
	//     });
	//
	//   // Subscribe to custom ASC events if available:
	//   if (UCoreAbilitySystemComponent* CoreASC = Cast<UCoreAbilitySystemComponent>(AbilitySystemComponent))
	//   {
	//       CoreASC->OnEffectAssetTags.AddLambda([this](const FGameplayTagContainer& Tags) { ... });
	//   }
}