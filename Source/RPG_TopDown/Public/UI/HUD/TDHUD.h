// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Engine & Module Includes =====
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "TDHUD.generated.h"

// ===== Forward Declarations =====
class UTDHUDWidgetController;
class UTDAttributeMenuWidgetController;
class UTDUserWidget;
class UUserWidget;
class UAttributeSet;
class UAbilitySystemComponent;
struct FGASCoreUIWidgetControllerParams;

/**
 * ATDHUD
 *
 * Custom HUD class for GAS-driven games.
 * - Creates and owns UI widgets and their controllers.
 * - Provides accessors to retrieve or lazily create controllers with correct references.
 */
UCLASS()
class RPG_TOPDOWN_API ATDHUD : public AHUD
{
	GENERATED_BODY()

public:
	// ===== Controller accessors =====

	/**
	 * GetHUDWidgetController
	 * Returns a pointer to the HUD Widget Controller, creating and initializing it if none exists.
	 * @param InWidgetControllerParams   References required by the widget controller.
	 */
	UTDHUDWidgetController* GetHUDWidgetController(const FGASCoreUIWidgetControllerParams& InWidgetControllerParams);

	/**
	 * GetAttributeMenuWidgetController
	 * Returns the Attribute Menu controller, creating and initializing it if needed.
	 */
	UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FGASCoreUIWidgetControllerParams& InWidgetControllerParams);

	// ===== Initialization =====

	/**
	 * InitializeHUD
	 * Spawns the HUD widget, creates/initializes its controller, and adds the widget to the viewport.
	 */
	void InitializeHUD(APlayerController* InPlayerController,
		APlayerState* InPlayerState,
		UAbilitySystemComponent* InAbilitySystemComponent,
		UAttributeSet* InAttributeSet);

private:
	// ===== Widget instances =====

	/** On-screen HUD widget instance (your custom user widget). */
	UPROPERTY()
	TObjectPtr<UTDUserWidget> HUDWidget;

	/** Concrete class used to spawn the HUD widget at runtime (set in BP). */
	UPROPERTY(EditAnywhere, Category="UI|Classes")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/** HUD widget controller instance. Owned by this HUD. */
	UPROPERTY()
	TObjectPtr<UTDHUDWidgetController> HUDWidgetController;

	/** Concrete class used to instantiate the HUD widget controller (set in BP). */
	UPROPERTY(EditAnywhere, Category="UI|Classes")
	TSubclassOf<UTDHUDWidgetController> HUDWidgetControllerClass;

	/** Attribute Menu widget controller instance. */
	UPROPERTY()
	TObjectPtr<UTDAttributeMenuWidgetController> AttributeMenuWidgetController;

	/** Concrete class used to instantiate the Attribute Menu widget controller (set in BP). */
	UPROPERTY(EditAnywhere, Category="UI|Classes")
	TSubclassOf<UTDAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;
}
;