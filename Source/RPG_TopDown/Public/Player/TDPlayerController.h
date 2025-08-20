// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TDPlayerController.generated.h"

class UHighlightInteraction;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

/**
 * ATDPlayerController
 *
 * Custom player controller for the GAS TopDown RPG project.
 * - Handles enhanced input (via Enhanced Input Plugin).
 * - Owns and exposes highlight interaction logic.
 * - Sets up mouse and input modes.
 */
UCLASS()
class RPG_TOPDOWN_API ATDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ATDPlayerController();

protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Component handling highlighting logic for the player (e.g. interactable outlines).
	  * Automatically created and registered as a subobject.
	  * Exposed to Blueprints for easy access from UI or interaction systems.
	  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Highlight")
	TObjectPtr<UHighlightInteraction> HighlightInteraction;


private:

	/** Input mapping context asset, providing the set of input actions for this controller.
	  * Must be set in Blueprint or defaults for input to function.
	  */
	UPROPERTY(EditAnywhere, Category = "Enhanced Input")
	TObjectPtr<UInputMappingContext> GASInputMappingContext;

	/** Move input action asset.
	  * Assigned in Blueprint or defaults, defines the move input binding.
	  */
	UPROPERTY(EditAnywhere, Category = "Enhanced Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Handler function for movement input.
	  * Called by Enhanced Input system when MoveAction is triggered.
	  * @param InputActionValue The value (axis/amount) of the input action.
	  */
	void Move(const FInputActionValue& InputActionValue);
};
