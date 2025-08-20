// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Player/TDPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interaction/HighlightInteraction.h"

ATDPlayerController::ATDPlayerController()
{
	// Set this controller to replicate (so it exists on server and all clients).
	bReplicates = true;

	// Create the highlight interaction component as a default subobject.
	// This allows Blueprint/UI to handle highlighting (e.g. mouseover outlines).
	HighlightInteraction = CreateDefaultSubobject<UHighlightInteraction>(TEXT("HighlightInteraction"));
}

void ATDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Early assert to catch missing input mapping context setup in BP/defaults.
	checkf(GASInputMappingContext, TEXT("GASPlayerController -> GASInputContext ptr is null!"));

	// Only local player controllers have a valid ULocalPlayer pointer.
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if(LocalPlayer)
	{
		// Get the Enhanced Input subsystem for this local player.
		UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSystem && GASInputMappingContext)
		{
			// Register the input mapping context (priority 0 = highest).
			InputSystem->AddMappingContext(GASInputMappingContext, 0);
		}
	}

	// Enable mouse cursor for top-down/RTS control.
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	// Set input mode to Game and UI (allows both gameplay and UI input).
	FInputModeGameAndUI InputModeData;
	// Do not lock mouse to viewport (allows dragging out for multi-monitor).
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// Show mouse cursor even when capturing input (for top-down feel).
	InputModeData.SetHideCursorDuringCapture(false);
	// Apply the input mode settings.
	SetInputMode(InputModeData);
}

void ATDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Cast to EnhancedInputComponent (required for Enhanced Input bindings).
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// Bind the move action (must be set in BP/defaults) to the Move handler on this controller.
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
}

void ATDPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// Get the 2D input vector (e.g., from WASD or analog stick).
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	// Get the current controller rotation (camera facing the direction).
	const FRotator Rotation = GetControlRotation();
	// Only use yaw for movement (ignore pitch/roll for top-down).
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.0f);

	// Calculate the forward direction (X axis) based-on-camera yaw.
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// Calculate the right direction (Y axis), also relative to camera yaw.
	const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

	// Get the pawn currently possessed by this player controller.
	APawn* ControlledPawn = GetPawn<APawn>();
	if (ControlledPawn)
	{
		// Move forward/backward based on Y axis of input (W/S or up/down on stick).
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		// Move right/left based on X axis of input (A/D or left/right on stick).
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
