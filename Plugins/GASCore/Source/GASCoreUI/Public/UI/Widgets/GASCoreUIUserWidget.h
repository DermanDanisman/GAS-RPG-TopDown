// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// UI MVC PATTERN - VIEW LAYER:
//   This class represents the View in our UI Model-View-Controller architecture.
//
// PATTERN OVERVIEW:
//   - MODEL: Gameplay Ability System (AbilitySystemComponent + AttributeSet + PlayerState)
//     * Authoritative game state and business logic
//     * Emits change notifications when state updates
//
//   - CONTROLLER: UGASCoreUIWidgetController and derived classes
//     * Bridges Model and View layers  
//     * Subscribes to Model change events, rebroadcasts to View
//     * Processes View input and routes to appropriate Model methods
//
//   - VIEW: UGASCoreUIUserWidget and derived Blueprint/C++ widgets (this class)
//     * Pure presentation layer focused on display and user interaction
//     * No direct access to gameplay systems (all data flows through Controller)
//     * Binds to Controller delegates to receive reactive state updates
//
// CONTROLLER INTEGRATION:
//   The handoff process ensures consistent initialization timing:
//   1) External code (HUD) creates and configures Controller with Model references
//   2) External code calls SetWidgetController() to establish View→Controller link  
//   3) OnWidgetControllerSet fires immediately, allowing Blueprint setup
//   4) Blueprint binds to Controller delegates (OnHealthChanged, etc.)
//   5) Controller subscribes to Model events and begins forwarding updates
//
// BLUEPRINT RESPONSIBILITIES:
//   Blueprint implementers should use OnWidgetControllerSet to:
//   - Cast WidgetController to appropriate concrete type (UCoreHUDWidgetController, etc.)
//   - Bind to Controller's BlueprintAssignable delegates for reactive updates
//   - Initialize UI elements with current state (or trigger Controller.BroadcastInitialValues())
//   - Set up any UI-specific configuration that depends on Controller presence
//
// LIFETIME CONSIDERATIONS:
//   - View does not own the Controller; external systems manage Controller lifetime
//   - Controller typically outlives individual Views (shared across widget instances)
//   - If Controller becomes invalid, View should gracefully degrade (show defaults, disable features)
//   - Consider null-checking Controller before accessing methods/properties
//
// ERROR HANDLING PATTERNS:
//   - Null Controller: Show placeholder UI, log warning, disable interactive elements
//   - Wrong Controller Type: Cast failure should disable specific features, not crash widget
//   - Delegate Binding Failures: Individual UI elements may not update, but core widget functionality continues
//
// NETWORKING NOTES:
//   - Controller typically runs on owning client (UI is client-side presentation)
//   - Model data replicates from server and drives Controller→View updates
//   - User input from View should route through Controller to appropriate server RPCs when needed
//
// See also:
//   - UI/WidgetControllers/CoreWidgetController.* (base Controller implementation)
//   - UI/WidgetControllers/CoreHUDWidgetController.* (example concrete Controller)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GASCoreUIUserWidget.generated.h"

/**
 * UGASCoreUIUserWidget
 *
 * Base class for all GAS TopDown RPG user widgets that integrate with the Controller layer.
 * Implements the View side of our UI MVC pattern with a standardized Controller handoff process.
 * 
 * CONTROLLER INTEGRATION:
 * - Provides SetWidgetController() for establishing the Controller→View relationship
 * - Fires OnWidgetControllerSet when controller is assigned (Blueprint hook for initialization)
 * - Maintains weak reference to controller (does not manage controller lifetime)
 * 
 * BLUEPRINT USAGE:
 * - Override OnWidgetControllerSet to bind to controller delegates
 * - Cast GetWidgetController() to concrete controller type for specific functionality
 * - Handle null controller gracefully (show defaults, disable features)
 * 
 * TYPICAL BLUEPRINT FLOW:
 * 1. On Widget Controller Set → Cast to UCoreHUDWidgetController
 * 2. Bind to OnHealthChanged → Update Progress Bar
 * 3. Bind to OnManaChanged → Update Progress Bar  
 * 4. Bind to MessageWidgetRowDelegate → Display Notifications
 */
UCLASS()
class GASCOREUI_API UGASCoreUIUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Returns the associated Widget Controller object (may be nullptr if not set).
	 * 
	 * USAGE NOTES:
	 * - This returns a raw UObject pointer; cast to your concrete controller class in BP/C++
	 * - The widget does not own the controller; external systems manage controller lifetime
	 * - Always null-check the result before use, especially in networked scenarios
	 * 
	 * BLUEPRINT PATTERN:
	 * - Cast to UCoreHUDWidgetController (or appropriate derived class)
	 * - Check "Is Valid" on the cast result before accessing controller methods
	 * - If cast fails, disable related UI features gracefully
	 */
	UFUNCTION(BlueprintPure, Category = "GASCore|User Widget|Widget Controller")
	UObject* GetAssociatedWidgetController() { return WidgetController; }

	/**
	 * Assigns the Widget Controller and triggers the Blueprint initialization event.
	 * 
	 * INITIALIZATION TIMING:
	 * Call this after the controller is constructed and its references are properly initialized.
	 * This should happen before any UI updates are expected, but after the widget is ready to bind delegates.
	 * 
	 * BLUEPRINT INTEGRATION:
	 * After assignment, OnWidgetControllerSet fires immediately, giving Blueprint designers
	 * a reliable hook for setting up delegate bindings and initial UI state.
	 * 
	 * MULTIPLE CALLS:
	 * This method can be called multiple times (e.g., pawn possession changes, level transitions).
	 * Blueprint implementations should ensure their binding logic handles repeat calls gracefully.
	 * 
	 * COMMON BLUEPRINT TASKS IN OnWidgetControllerSet:
	 * - Cast Widget Controller to concrete type (UCoreHUDWidgetController)
	 * - Bind to controller delegates (OnHealthChanged, OnManaChanged, etc.)
	 * - Initialize progress bars, text displays with current values
	 * - Set up any controller-dependent UI features
	 */
	UFUNCTION(BlueprintCallable, Category = "GASCore|User Widget|Widget Controller")
	virtual void SetWidgetController(UObject* InWidgetController);

protected:
	/**
	 * Blueprint event called immediately after the Widget Controller is assigned.
	 * 
	 * BLUEPRINT IMPLEMENTATION RESPONSIBILITIES:
	 * This is the primary hook for Blueprint designers to set up the UI→Controller relationship:
	 * 
	 * 1. CONTROLLER TYPE CASTING:
	 *    Cast the Widget Controller to the appropriate concrete class:
	 *    - UCoreHUDWidgetController for main HUD elements
	 *    - Custom controllers for specialized UI (inventory, dialogue, etc.)
	 * 
	 * 2. DELEGATE BINDING:
	 *    Bind to the controller's BlueprintAssignable delegates:
	 *    - OnHealthChanged → Update Health Progress Bar
	 *    - OnManaChanged → Update Mana Progress Bar  
	 *    - MessageWidgetRowDelegate → Display Notifications/Toasts
	 * 
	 * 3. INITIAL UI STATE:
	 *    If needed, trigger initial value broadcasts:
	 *    - Call controller.BroadcastInitialValues() if not done elsewhere
	 *    - Set default UI state for elements not driven by delegates
	 * 
	 * ERROR HANDLING:
	 * - If controller cast fails, disable related UI features and log warning
	 * - If delegate binding fails, individual elements may not update (graceful degradation)
	 * - Always check controller validity before accessing methods/properties
	 * 
	 * IDEMPOTENCY CONSIDERATION:
	 * This event fires every time SetWidgetController is called. Design your binding logic to:
	 * - Handle multiple calls gracefully (unbind previous delegates before rebinding)
	 * - Or use binding patterns that naturally handle duplicates
	 * - Or guard with a "binding complete" flag if appropriate
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GASCore|User Widget|Widget Controller", meta = (DisplayName = "On Widget Controller Set"))
	void OnWidgetControllerSet();

private:
	/**
	 * Reference to the Widget Controller object that serves as the data source for this widget.
	 * 
	 * LIFETIME MANAGEMENT:
	 * - Not owned by this widget; external systems (typically HUD) manage controller lifetime
	 * - Controller typically outlives individual widget instances (shared across UI elements)
	 * - Should remain valid for the duration of the widget's active lifecycle
	 * 
	 * ACCESS PATTERN:
	 * - Not exposed directly to Blueprint to encourage explicit access via GetWidgetController()
	 * - This makes the dependency relationship clear in Blueprint graphs
	 * - Enables easier debugging of controller reference issues
	 * 
	 * NULL SAFETY:
	 * - Can legitimately be null during widget construction or teardown phases
	 * - Blueprint logic should always verify controller validity before use
	 * - Consider graceful degradation when controller becomes unavailable
	 */
	UPROPERTY()
	TObjectPtr<UObject> WidgetController = nullptr;
};