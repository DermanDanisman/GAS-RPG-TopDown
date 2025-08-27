// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// CONTROLLER/VIEW CONTRACT IMPLEMENTATION NOTES:
//
// This class establishes the View side of our UI MVC pattern:
// - VIEW RESPONSIBILITIES: Display data, handle user input, remain decoupled from business logic
// - CONTROLLER DEPENDENCIES: View depends on Controller for all data and state changes
// - CONTRACT ENFORCEMENT: OnWidgetControllerSet ensures consistent initialization timing
//
// CONTROLLER HANDOFF LIFECYCLE:
// 1) HUD creates and configures controller with gameplay system references
// 2) HUD calls Widget.SetWidgetController(controller) to establish the link
// 3) SetWidgetController stores reference and immediately fires OnWidgetControllerSet
// 4) Blueprint implements OnWidgetControllerSet to:
//    a) Cast controller to specific type (e.g., UCoreHUDWidgetController)
//    b) Bind to controller's BlueprintAssignable delegates
//    c) Set up initial UI state if needed
//
// BLUEPRINT EVENT TIMING:
// OnWidgetControllerSet fires every time SetWidgetController is called, not just the first time.
// This supports scenarios where:
// - Pawn possession changes require new controller references
// - Level transitions need fresh controller instances
// - Hot-reloading or debug scenarios reset controller links
//
// LIFETIME CAVEATS:
// - Widget does not own the controller; external code manages controller lifetime
// - If controller is destroyed while widget exists, calls through WidgetController will crash
// - In networked games, ensure server authority for controller data remains consistent
// - Consider weak pointers or validity checks if controller lifetime is uncertain
//
// USAGE PATTERN EXAMPLE:
// In Blueprint OnWidgetControllerSet:
//   1. Cast Widget Controller to UCoreHUDWidgetController
//   2. Bind to OnHealthChanged → Update Health Bar
//   3. Bind to OnManaChanged → Update Mana Bar  
//   4. Bind to MessageWidgetRowDelegate → Show Notifications
//
// ERROR HANDLING:
// - If controller is null, widget should gracefully degrade (show default values, disable interactions)
// - If controller cast fails, log warning and disable related UI features
// - If delegate binding fails, individual UI elements may not update but widget shouldn't crash

#include "GASCore/Public/UI/Widgets/CoreUserWidget.h"

void UCoreUserWidget::SetWidgetController(UObject* InWidgetController)
{
	// CONTROLLER REFERENCE HANDOFF:
	// Store the controller reference (not owned by this widget).
	// This enables the widget to access controller methods and bind to its delegates.
	WidgetController = InWidgetController;

	// BLUEPRINT INITIALIZATION HOOK:
	// Fire the Blueprint event immediately after assignment.
	// This gives Blueprint designers a reliable, consistent point to set up UI bindings.
	//
	// IMPORTANT: This event fires every time SetWidgetController is called.
	// Blueprint implementers should ensure their binding logic is idempotent or
	// guards against duplicate bindings if this is called multiple times.
	OnWidgetControllerSet();
}