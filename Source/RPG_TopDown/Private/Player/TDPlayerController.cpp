// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Player/TDPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"              // UEnhancedInputLocalPlayerSubsystem
#include "TDGameplayTags.h"
#include "AbilitySystem/Components/TDAbilitySystemComponent.h"
#include "Components/ClickToMoveComponent.h"
#include "Input/TDEnhancedInputComponent.h"       // UTDEnhancedInputComponent for binding with tags
#include "Input/TDInputConfig.h"
#include "Interaction/HighlightInteraction.h"     // UHighlightInteraction

ATDPlayerController::ATDPlayerController()
{
	// Replicate this controller so it exists on server and relevant clients.
	bReplicates = true;

	// Create the highlight interaction component as a default subobject.
	// Enables BP/UI to manage interactable highlighting (e.g., outlines on hover).
	HighlightInteraction = CreateDefaultSubobject<UHighlightInteraction>(TEXT("HighlightInteraction"));
	
	ClickToMoveComponent = CreateDefaultSubobject<UClickToMoveComponent>(TEXT("ClickToMoveComponent"));
	
}

void ATDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only local player controllers have a valid ULocalPlayer pointer.
	// Mapping contexts are a client/local concern (do not assert on server).
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		// Early assert to catch missing input mapping context setup in BP/defaults.
		checkf(GASInputMappingContext, TEXT("ATDPlayerController: GASInputMappingContext is null on [%s]. Set it in defaults/BP."), *GetNameSafe(this));

		// Get the Enhanced Input subsystem for this local player and register the mapping context.
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			// Priority 0 (higher is higher priority, but 0 is fine for a primary context).
			InputSystem->AddMappingContext(GASInputMappingContext, /*Priority=*/0);
		}
	}

	// Configure mouse for top-down/RTS-like control.
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
	
	// Cast to our Enhanced Input component. Will crash if the component type is different (by design).
	UTDEnhancedInputComponent* TDEnhancedInputComponent = CastChecked<UTDEnhancedInputComponent>(InputComponent);

	// Bind the move action (must be set in BP/defaults) to the Move handler on this controller.
	if (ensureMsgf(MoveAction != nullptr, TEXT("ATDPlayerController: MoveAction is null. Set it in defaults/BP.")))
	{
		TDEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	}

	// Bind all ability input actions (Pressed/Released/Held) using the data-driven input config.
	if (ensureMsgf(InputConfig != nullptr, TEXT("ATDPlayerController: InputConfig is null. Set it in defaults/BP.")))
	{
		TDEnhancedInputComponent->BindAbilityInputActions(
			InputConfig,
			this,
			&ThisClass::AbilityInputActionTagPressed,
			&ThisClass::AbilityInputActionReleased,
			&ThisClass::AbilityInputActionHeld
		);
	}
}

void ATDPlayerController::Move(const FInputActionValue& InputActionValue)
{
	ClickToMoveComponent->SetAutoRunActive(false);
	
	// Get the 2D input vector (e.g., from WASD or analog stick).
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	// Use only yaw for movement orientation (ignore pitch/roll for top-down).
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.0f);

	// Build forward (X) and right (Y) vectors from yaw-only rotation.
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Get the pawn currently possessed by this player controller.
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// Move forward/backward based on Y axis of input (W/S or up/down on stick).
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		// Move right/left based on X axis of input (A/D or left/right on stick).
		ControlledPawn->AddMovementInput(RightDirection,   InputAxisVector.X);
	}
}

UTDAbilitySystemComponent* ATDPlayerController::GetASC()
{
	if (TDAbilitySystemComponent == nullptr)
	{
		TDAbilitySystemComponent = Cast<UTDAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return TDAbilitySystemComponent;
}

void ATDPlayerController::AbilityInputActionTagPressed(const FGameplayTag InputTag)
{
	// LMB is shared by abilities and movement; gate movement when targeting.
	if (InputTag.MatchesTagExact(FTDGameplayTags::Get().InputTag_LMB))
	{
		const bool bIsTargeting = HighlightInteraction && (HighlightInteraction->GetHighlightedActor() != nullptr);
		ClickToMoveComponent->SetIsTargeting(bIsTargeting);
		ClickToMoveComponent->OnClickPressed();
	}
}

void ATDPlayerController::AbilityInputActionReleased(const FGameplayTag InputTag)
{
	// Non-LMB: forward to ASC.
	if (!InputTag.MatchesTagExact(FTDGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	// LMB: if targeting, forward to ASC; otherwise finalize click-to-move (build a path on short press).
	if (HighlightInteraction->GetHighlightedActor())
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
	}
	else
	{
		ClickToMoveComponent->OnClickReleased();
	}
}

void ATDPlayerController::AbilityInputActionHeld(const FGameplayTag InputTag)
{
	// Non-LMB: forward to ASC.
	if (!InputTag.MatchesTagExact(FTDGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// LMB: when targeting, let ASC handle. Otherwise, let ClickToMove do its own NAVIGATION-channel trace.
	if (HighlightInteraction->GetHighlightedActor())
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		// Use internal nav-channel trace to get a ground point (avoids mixing highlight hits with nav hits).
		ClickToMoveComponent->OnClickHeld(/*bUseInternalHitResult=*/true, FHitResult());
	}
}