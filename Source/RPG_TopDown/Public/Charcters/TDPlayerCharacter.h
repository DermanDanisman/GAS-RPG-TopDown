// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "TDCharacterBase.h"
#include "TDPlayerCharacter.generated.h"

/**
 * ATDPlayerCharacter
 *
 * Primary gameplay character class for the GAS TopDown RPG project.
 * - Inherits from ATDCharacterBase, which provides GAS integration.
 * - Handles movement, possession, and GAS initialization for both server and clients.
 *
 * GAS ownership:
 * - For players, the ASC and AttributeSet live on the PlayerState (authoritative, persistent).
 * - This character becomes the AvatarActor for the PlayerState's ASC.
 */
UCLASS()
class RPG_TOPDOWN_API ATDPlayerCharacter : public ATDCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDPlayerCharacter();

	/** Called when this character is possessed by a new controller (server-side).
	  * Initializes the Ability System with the new controller and state.
	  * @param NewController The controller (player or AI) that now possesses this character.
	  */
	virtual void PossessedBy(AController* NewController) override;

	/** Called when the replicated PlayerState is updated (client-side).
	  * Ensures Ability System is re-initialized for the new PlayerState.
	  */
	virtual void OnRep_PlayerState() override;

	/** Combat Interface: get level from PlayerState for player-controlled characters. */
	virtual int32 GetActorLevel() override;
	
protected:

	/** Initialize GAS owner/avatar references (PlayerState owner, this character as avatar). */
	virtual void InitializeAbilityActorInfo() override;
};