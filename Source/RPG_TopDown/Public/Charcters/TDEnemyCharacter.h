// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDCharacterBase.h"
#include "Interaction/HighlightInterface.h"
#include "TDEnemyCharacter.generated.h"

/**
 * ATDEnemyCharacter
 *
 * Enemy character class with GAS (Gameplay Ability System) and highlight interaction.
 * Inherits from ATDCharacterBase (provides Ability System, Attribute Set) and supports IHighlightInterface.
 */
UCLASS()
class RPG_TOPDOWN_API ATDEnemyCharacter : public ATDCharacterBase, public IHighlightInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDEnemyCharacter();

	/** Sets the actor as highlighted (visual feedback, e.g. for mouseover). */
	virtual void HighlightActor() override;

	/** Clears the highlight state from the actor. */
	virtual void UnHighlightActor() override;

	/** Whether this enemy is currently highlighted (visible to Blueprints/UI). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interactable)
	bool bHighlighted;

protected:

	virtual void BeginPlay() override;

	virtual void InitializeAbilityActorInfo() override;
};
