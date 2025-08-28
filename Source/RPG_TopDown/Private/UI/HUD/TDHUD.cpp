// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "UI/HUD/TDHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/WidgetControllers/TDHUDWidgetController.h"
#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"
#include "UI/Widgets/TDUserWidget.h"


UTDHUDWidgetController* ATDHUD::GetHUDWidgetController(const FGASCoreUIWidgetControllerParams& InWidgetControllerParams)
{
	// If the controller hasn't been created yet, instantiate it and initialize.
	if (HUDWidgetController == nullptr)
	{
		// Create a new widget controller of the specified class, owned by this HUD.
		HUDWidgetController = NewObject<UTDHUDWidgetController>(this, HUDWidgetControllerClass);

		// Initialize the widget controller with all gameplay references.
		HUDWidgetController->SetWidgetControllerParams(InWidgetControllerParams);

		HUDWidgetController->BindCallbacksToDependencies();

		// Return the newly created and initialized controller.
		return HUDWidgetController;
	}
	// If already created, just return the existing controller.
	return HUDWidgetController;
}

UTDAttributeMenuWidgetController* ATDHUD::GetAttributeMenuWidgetController(const FGASCoreUIWidgetControllerParams& InWidgetControllerParams)
{
	// If the controller hasn't been created yet, instantiate it and initialize.
	if (AttributeMenuWidgetController == nullptr)
	{
		// Create a new widget controller of the specified class, owned by this HUD.
		AttributeMenuWidgetController = NewObject<UTDAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);

		// Initialize the widget controller with all gameplay references.
		AttributeMenuWidgetController->SetWidgetControllerParams(InWidgetControllerParams);

		AttributeMenuWidgetController->BindCallbacksToDependencies();

		// Return the newly created and initialized controller.
		return AttributeMenuWidgetController;
	}
	// If already created, just return the existing controller.
	return AttributeMenuWidgetController;
}

void ATDHUD::InitializeHUD(APlayerController* InPlayerController, APlayerState* InPlayerState,
                           UAbilitySystemComponent* InAbilitySystemComponent, UAttributeSet* InAttributeSet)
{
	// Ensure that the widget and controller classes are set in the Blueprint or C++.
	// If not, crash with a clear message so the developer/designer can fix the setup.
	checkf(HUDWidgetClass, TEXT("HUD Widget Class uninitialized, please fill out BP_TDHUD"));
	checkf(HUDWidgetControllerClass, TEXT("HUD Widget Controller Class uninitialized, please fill out BP_TDHUD"));
	checkf(AttributeMenuWidgetControllerClass, TEXT("Attribute Menu Widget Controller Class uninitialized, please fill out BP_TDHUD"));

	// Create the HUD widget instance using the specified class.
	// This creates a UUserWidget, which we then cast to our custom UGASUserWidget class.
	UUserWidget* UserWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);

	// Safely cast the generic UserWidget to your GAS-specific widget type.
	// This allows you to call GAS-specific functions or bind to GAS-specific data.
	HUDWidget = Cast<UTDUserWidget>(UserWidget);

	// Compose all relevant gameplay references (controller, state, ability system, attributes)
	// into a single struct for easier, cleaner parameter passing.
	const FGASCoreUIWidgetControllerParams WidgetControllerParams(
		InPlayerController, 
		InPlayerState, 
		InAbilitySystemComponent, 
		InAttributeSet
	);

	// Retrieve or create the HUD widget controller, initializing it with references if necessary.
	// This ensures the controller always has up-to-date references, regardless of reuse.
	UTDHUDWidgetController* LocalHUDWidgetController = GetHUDWidgetController(WidgetControllerParams);

	// Assign the controller to the widget.
	// This is the key to connecting your UI logic (controller) to your UI representation (widget).
	// From now on, the widget can access the controller for GAS data and logic.
	HUDWidget->SetWidgetController(LocalHUDWidgetController);

	HUDWidgetController->BroadcastInitialValues();

	// Finally, add the widget to the viewport so it becomes visible to the player.
	// This is the step that actually displays the UI on screen.
	UserWidget->AddToViewport();
}
