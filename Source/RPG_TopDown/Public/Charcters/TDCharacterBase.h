// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TDCharacterBase.generated.h"

class UTDDefaultAttributeInitComponent;
// Forward declaration to minimize dependencies. AttributeSet is used as a pointer member.
class UAttributeSet;

/**
 * ATDCharacterBase
 *
 * Abstract base character class implementing the AbilitySystemInterface for GAS (Gameplay Ability System).
 * - Inherit from this class for both player and AI characters that require ability system integration.
 * - Handles setup of the ability system and attribute set references, and provides accessors for UI/controllers.
 */
UCLASS()
class RPG_TOPDOWN_API ATDCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDCharacterBase();

	/** Returns the owned Ability System Component (ASC) for GAS integration.
	  * Required override for IAbilitySystemInterface.
	  */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Returns the owned Attribute Set for this character (used for GAS stats).
	  * Not part of the GAS interface, but commonly used for convenience.
	  */
	virtual UAttributeSet* GetAttributeSet() const;

protected:
	
	virtual void BeginPlay() override;

	/** Initializes all GAS-related references and connects them to the correct actors/components.
	  * Called by subclasses or externally after possession/state acquisition.
	  * Handles both player-controlled and AI characters.
	  */
	virtual void InitializeAbilityActorInfo();

	/** Skeletal mesh for the character's weapon.
	  * Exposed to the editor for easy assignment and setup.
	  */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	/** The Ability System Component for this actor.
	  * May be owned by the PlayerState (for players) or by the character (for AI).
	  */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** The AttributeSet for this actor, containing all modifiable gameplay stats. */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTDDefaultAttributeInitComponent> DefaultAttributeInitComponent;
};
