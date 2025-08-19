// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// MVC PATTERN IMPLEMENTATION NOTES:
// This base controller implements the Controller layer in our UI MVC architecture:
// 
// MODEL: Gameplay Ability System (AbilitySystemComponent + AttributeSet)
//  - Contains the authoritative game state (attributes, abilities, effects)
//  - Emits change notifications via delegates when state changes
//  - Read-only from the UI perspective (controller mediates modifications)
//
// VIEW: UUserWidget-derived Blueprint/C++ widgets  
//  - Pure display layer, no direct access to gameplay systems
//  - Binds to controller delegates to receive state updates
//  - Sends user input back to controller (which routes to appropriate systems)
//
// CONTROLLER: UCoreWidgetController (this class)
//  - Bridges Model and View layers
//  - Subscribes to Model change events, rebroadcasts to View
//  - Processes View input and calls appropriate Model methods
//  - Maintains references to all necessary gameplay systems
//
// INITIALIZATION ORDER:
// The controller follows a specific initialization sequence to ensure all dependencies are available:
// 1) Create controller instance (C++ or Blueprint)
// 2) SetWidgetControllerParams(params) - establishes Model references
// 3) Widget.SetWidgetController(controller) - establishes View→Controller link
//    - This triggers OnWidgetControllerSet in Blueprint, where View binds to Controller delegates
// 4) BindCallbacksToDependencies() - Controller subscribes to Model changes
// 5) BroadcastInitialValues() - Controller pushes current Model state to View
//
// DERIVED CONTROLLER RESPONSIBILITIES:
// - Override BroadcastInitialValues() to push current attribute values to UI on initialization
// - Override BindCallbacksToDependencies() to subscribe to ASC/AttributeSet changes and forward them
// - Expose BlueprintAssignable delegates that widgets can bind to for reactive updates
// - Handle MessageWidgetRowDelegate broadcasts for UI notifications (optional)
//
// LIFETIME MANAGEMENT:
// - Controller does not own the Model references (ASC, AttributeSet, etc.)
// - Ensure Model systems outlive the controller, or clear references before destruction
// - Use appropriate delegate binding strategies (AddLambda vs AddUObject) based on lifetime needs
//   - AddLambda: Convenient but requires manual cleanup if controller lifetime differs
//   - AddUObject: Automatic cleanup when UObject is destroyed, safer for most cases

#include "UI/WidgetControllers/CoreWidgetController.h"

void UCoreWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& InWidgetControllerParams)
{
	// DEPENDENCY INJECTION: Assign all gameplay system references at once
	// This should be the first call after constructing the controller, before any UI binding occurs.
	// 
	// PARAMETER VALIDATION:
	// While we don't enforce non-null here, derived classes should validate critical references
	// before subscribing to delegates or broadcasting values (use check() or ensure() as appropriate).
	PlayerController       = InWidgetControllerParams.PlayerController;
	PlayerState            = InWidgetControllerParams.PlayerState;
	AbilitySystemComponent = InWidgetControllerParams.AbilitySystemComponent;
	AttributeSet           = InWidgetControllerParams.AttributeSet;

	// Optional: add defensive checks in derived classes (e.g., ensure non-null ASC/AttributeSet).
	// Example:
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