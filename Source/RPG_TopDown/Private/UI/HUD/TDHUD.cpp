// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "UI/HUD/TDHUD.h"

// Engine
#include "Blueprint/UserWidget.h"

// Project
#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"
#include "UI/WidgetControllers/TDHUDWidgetController.h"
#include "UI/Widgets/TDUserWidget.h"

UTDHUDWidgetController* ATDHUD::GetHUDWidgetController(const FGASCoreUIWidgetControllerParams& InWidgetControllerParams)
{
	// Lazily create controller and bind delegates once.
	if (HUDWidgetController == nullptr)
	{
		HUDWidgetController = NewObject<UTDHUDWidgetController>(this, HUDWidgetControllerClass);

		// Provide all runtime references (PC/PS/ASC/AttributeSet).
		HUDWidgetController->SetWidgetControllerParams(InWidgetControllerParams);

		// Allow controller to subscribe to dependent events (ASC, etc.).
		HUDWidgetController->BindCallbacksToDependencies();
	}
	return HUDWidgetController;
}

UTDAttributeMenuWidgetController* ATDHUD::GetAttributeMenuWidgetController(
	const FGASCoreUIWidgetControllerParams& InWidgetControllerParams)
{
	// Lazily create Attribute Menu controller and bind delegates once.
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UTDAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);

		AttributeMenuWidgetController->SetWidgetControllerParams(InWidgetControllerParams);

		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	return AttributeMenuWidgetController;
}

void ATDHUD::InitializeHUD(APlayerController* InPlayerController, APlayerState* InPlayerState,
                           UAbilitySystemComponent* InAbilitySystemComponent, UAttributeSet* InAttributeSet)
{
	// Ensure that the widget and controller classes are configured (usually in BP).
	checkf(HUDWidgetClass, TEXT("HUD Widget Class uninitialized, please fill out BP_GASHUD"));
	checkf(HUDWidgetControllerClass, TEXT("HUD Widget Controller Class uninitialized, please fill out BP_GASHUD"));

	// Create the concrete UUserWidget instance using the configured class.
	UUserWidget* UserWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);

	// Cast to your GAS-specific widget type for controller assignment and GAS-aware behavior.
	HUDWidget = Cast<UTDUserWidget>(UserWidget);

	// Compose all relevant references into a single params struct.
	const FGASCoreUIWidgetControllerParams WidgetControllerParams(
		InPlayerController,
		InPlayerState,
		InAbilitySystemComponent,
		InAttributeSet
	);

	// Retrieve or create the HUD widget controller.
	UTDHUDWidgetController* LocalHUDWidgetController = GetHUDWidgetController(WidgetControllerParams);

	// Connect the widget with its controller (MVC-style).
	HUDWidget->SetWidgetController(LocalHUDWidgetController);

	// Push initial values to the HUD after binding.
	HUDWidgetController->BroadcastInitialValues();

	// Finally, add the widget to the viewport so it becomes visible.
	UserWidget->AddToViewport();
}