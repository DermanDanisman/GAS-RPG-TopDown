// Fill out your copyright notice in the Description page of Project Settings.

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
	
protected:

	virtual void BeginPlay() override;

	virtual void InitializeAbilityActorInfo() override;
};
