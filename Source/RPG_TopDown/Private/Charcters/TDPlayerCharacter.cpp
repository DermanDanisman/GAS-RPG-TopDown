// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Charcters/TDPlayerCharacter.h"

#include "GASCore/Public/Attributes/CoreAttributeSet.h"
#include "Components/TDAbilitySystemComponent.h"
#include "Components/TDDefaultAttributeInitComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/TDPlayerController.h"
#include "Player/TDPlayerState.h"
#include "UI/HUD/TDHUD.h"

// Sets default values
ATDPlayerCharacter::ATDPlayerCharacter()
{
	// Orient rotation to the movement direction (character faces where it moves).
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Set the rotation rate for smooth turning.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	// Lock movement to a plane (X/Y for top-down).
	GetCharacterMovement()->bConstrainToPlane = true;
	// Snap the character to the movement plane at spawn.
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Do not use controller rotation for pitch, yaw, or roll (handled by movement).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void ATDPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize Ability System Actor Info for the server context (PlayerState owner, this avatar).
	InitializeAbilityActorInfo();
}

void ATDPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Initialize Ability System Actor Info for the client context (after PlayerState replicated).
	InitializeAbilityActorInfo();
}

int32 ATDPlayerCharacter::GetActorLevel()
{
	// Get the custom PlayerState class.
	const ATDPlayerState* TDPlayerState = GetPlayerState<ATDPlayerState>();
	check(TDPlayerState); // Should exist when ASC is initialized.
	return TDPlayerState->GetPlayerLevel();
}

void ATDPlayerCharacter::InitializeAbilityActorInfo()
{
	if (AController* CharacterController = GetController())
	{
		if (CharacterController->IsPlayerController())
		{
			// --- Player-controlled character setup ---

			// Get the custom PlayerState class.
			ATDPlayerState* TDPlayerState = GetPlayerState<ATDPlayerState>();
			if (TDPlayerState)
			{
				// The ASC and AttributeSet live on the PlayerState for player characters.
				AbilitySystemComponent = Cast<UTDAbilitySystemComponent>(TDPlayerState->GetAbilitySystemComponent());
				AttributeSet = Cast<UCoreAttributeSet>(TDPlayerState->GetAttributeSet());

				// Initialize the ASC's actor info, specifying the player state as the owner and this character as the avatar.
				if (AbilitySystemComponent)
				{
					AbilitySystemComponent->InitAbilityActorInfo(TDPlayerState, this);

					// Bind ASC delegates (attribute changes, tag changes, etc.) if your ASC subclass exposes them.
					Cast<UTDAbilitySystemComponent>(TDPlayerState->GetAbilitySystemComponent())->BindASCDelegates();
				}
			}

			// In multiplayer, only the locally controlled character on each client will have a valid PlayerController pointer.
			// For other (non-local) characters on that client, PlayerController will be nullptr. This is expected.
			ATDPlayerController* TDPlayerController = Cast<ATDPlayerController>(CharacterController);
			if (TDPlayerController)
			{
				// Get the custom HUD and initialize it with all gameplay references.
				ATDHUD* TDHUD = Cast<ATDHUD>(TDPlayerController->GetHUD());
				if (TDHUD)
				{
					TDHUD->InitializeHUD(TDPlayerController, TDPlayerState, AbilitySystemComponent, AttributeSet);
				}
			}

			// Apply default attribute initialization once ASC and AttributeSet are valid.
			// DefaultAttributeInitComponent is assumed to be provided by ATDCharacterBase.
			if (AbilitySystemComponent && AttributeSet && DefaultAttributeInitComponent->DefaultPrimaryAttributes)
			{
				DefaultAttributeInitComponent->InitializeDefaultAttributes(AbilitySystemComponent);
			}
		}
	}
}