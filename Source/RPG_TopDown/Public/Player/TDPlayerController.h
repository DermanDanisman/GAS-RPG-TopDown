// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "TDPlayerController.generated.h"

class UTDInputConfig;
class UHighlightInteraction;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

/**
 * ATDPlayerController
 *
 * Custom player controller for the GAS TopDown RPG project.
 * Responsibilities:
 * - Enhanced Input setup (via Input Mapping Context and Input Actions).
 * - Mouse/viewport input mode configuration for top-down controls.
 * - Exposes a highlight interaction component for interactable feedback (e.g., outlines).
 *
 * GAS Input:
 * - Ability input binding is data-driven via UTDInputConfig. Input actions are mapped to FGameplayTag
 *   and are forwarded to handler functions on this controller (Pressed/Released/Held).
 */
UCLASS()
class RPG_TOPDOWN_API ATDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Default constructor. Sets replication and creates highlight interaction component. */
	ATDPlayerController();

protected:
	// ===== APlayerController overrides =====

	/** Initializes input mapping contexts and mouse/input mode for local players. */
	virtual void BeginPlay() override;

	/** Binds Enhanced Input actions to local handler functions. */
	virtual void SetupInputComponent() override;

	// ===== Components =====

	/** Component handling highlighting logic for the player (e.g., interactable outlines).
	  * Automatically created and registered as a subobject.
	  * Exposed to Blueprints for easy access from UI or interaction systems.
	  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Highlight")
	TObjectPtr<UHighlightInteraction> HighlightInteraction;

private:
	// ===== Enhanced Input assets and config =====

	/** Input mapping context asset, providing the set of input actions for this controller.
	  * Must be set in Blueprint or defaults for input to function on the local player.
	  */
	UPROPERTY(EditAnywhere, Category="Enhanced Input")
	TObjectPtr<UInputMappingContext> GASInputMappingContext = nullptr;

	/** Move input action asset.
	  * Assigned in Blueprint or defaults, defines the move input binding (e.g., WASD/Stick).
	  */
	UPROPERTY(EditAnywhere, Category="Enhanced Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;

	/** Data asset mapping ability-related input actions to gameplay tags.
	  * Used by Enhanced Input to bind ability input events (Pressed/Released/Held) to handler functions.
	  */
	UPROPERTY(EditAnywhere, Category="Enhanced Input|Config")
	TObjectPtr<UTDInputConfig> InputConfig = nullptr;

	// ===== Input handlers =====

	/** Handler function for movement input.
	  * Called by Enhanced Input system when MoveAction is triggered.
	  * @param InputActionValue The value (axis/amount) of the input action.
	  */
	void Move(const FInputActionValue& InputActionValue);

	/** Ability input handler for "Pressed" (ETriggerEvent::Started). */
	void AbilityInputActionTagPressed(FGameplayTag InputTag);

	/** Ability input handler for "Released" (ETriggerEvent::Completed). */
	void AbilityInputActionReleased(FGameplayTag InputTag);

	/** Ability input handler for "Held" (ETriggerEvent::Triggered). */
	void AbilityInputActionHeld(FGameplayTag InputTag);
};