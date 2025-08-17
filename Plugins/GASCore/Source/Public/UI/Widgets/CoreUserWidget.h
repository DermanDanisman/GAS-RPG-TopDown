// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Summary:
//   Base UUserWidget for widgets that interact with a Widget Controller object.
//   Establishes a small, consistent contract between widgets (View) and controllers (Controller)
//   in the UI MVC pattern used by this project.
//
// Pattern (UI MVC):
//   - Model: Gameplay Ability System (ASC + AttributeSet)
//   - Controller: UCoreWidgetController or derived classes (reads model, broadcasts updates)
//   - View: UCoreUserWidget or derived widgets (bind to controller delegates; display-only)
//
// Responsibilities:
//   - Hold a reference to a controller UObject (not owned).
//   - Provide SetWidgetController() for wiring in C++/Blueprint.
//   - Fire OnWidgetControllerSet (BlueprintImplementableEvent) after the controller is assigned,
//     so Blueprints can bind delegates and initialize UI safely.
//
// Usage:
//   - In your HUD/initialization code:
//       Widget->SetWidgetController(Controller);
//       // In BP on the widget, implement "On Widget Controller Set" to bind to controller delegates.
//   - Widgets should avoid querying the world directly; rely on the controller for data.
//
// Lifetime & Safety Notes:
//   - This widget does not own the controller; it stores a UObject reference only.
//   - Ensure the controller outlives the widget or clears the reference before destruction.
//   - Calling SetWidgetController multiple times will invoke OnWidgetControllerSet each time
//     (which can be useful when pawns/possession change).
//
// See also:
//   - UI/WidgetControllers/CoreWidgetController.* (base controller API)
//   - UI/WidgetControllers/CoreHUDWidgetController.* (example concrete controller)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreUserWidget.generated.h"

/**
 * UCoreUserWidget
 *
 * Base class for all GAS TopDown RPG user widgets that interact with a Widget Controller.
 * Provides a standard interface for associating a controller with the widget and a hook
 * (OnWidgetControllerSet) for Blueprints to initialize bindings/UI state.
 */
UCLASS()
class GASCORE_API UCoreUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Returns the associated Widget Controller object (may be nullptr if not set).
	 * Note:
	 * - This is a raw UObject pointer; cast in BP/C++ to your concrete controller class.
	 * - The widget does not own the controller; manage lifetime externally.
	 */
	UFUNCTION(BlueprintPure, Category = "GAS User Widget|Widget Controller")
	UObject* GetWidgetController() { return WidgetController; }

	/**
	 * Assigns the Widget Controller and triggers the Blueprint event for initialization/binding.
	 * Call this once the controller is constructed and its references are initialized.
	 *
	 * Common follow-up in BP (On Widget Controller Set):
	 * - Bind to controller delegates (e.g., OnHealthChanged, OnManaChanged)
	 * - Initialize UI state (progress bars, text values) using controller-provided data
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS User Widget|Widget Controller")
	void SetWidgetController(UObject* InWidgetController);

protected:
	/**
	 * Blueprint event called after the Widget Controller is set.
	 * Implement in BP to bind delegates and perform initial UI setup.
	 * DisplayName is customized for readability in Blueprint graphs.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS User Widget|Widget Controller", meta = (DisplayName = "On Widget Controller Set"))
	void OnWidgetControllerSet();

private:
	/**
	 * The object that serves as the Widget Controller for this widget.
	 * Typically a UCoreWidgetController-derived object (C++ or BP).
	 *
	 * Not exposed to BP directly to encourage access via GetWidgetController(),
	 * which makes intent explicit in graphs.
	 */
	UPROPERTY()
	TObjectPtr<UObject> WidgetController = nullptr;
};