// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

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
 *
 * Design:
 * - AI owns its own ASC/AttributeSet (unlike players whose ASC lives on PlayerState).
 * - Level typically lives on the character itself for AI. See GetActorLevel().
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

	/** Combat Interface: return this enemy's level (used by MMCs, scaling, etc.). */
	virtual int32 GetActorLevel() override;

protected:
	virtual void BeginPlay() override;

	/** Initialize GAS owner/avatar for AI (AI owns its own ASC/AttributeSet). */
	virtual void InitializeAbilityActorInfo() override;
	
	/** AI level value; typically server-only relevance for calculations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 EnemyCharacterLevel;
};