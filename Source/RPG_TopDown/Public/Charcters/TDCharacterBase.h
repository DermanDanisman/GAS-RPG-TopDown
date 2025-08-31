// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Engine & Module Includes =====
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"

// GASCore Interfaces
#include "GASCore/Public/Interfaces/GASCoreCombatInterface.h"

#include "TDCharacterBase.generated.h"

// ===== Forward Declarations =====
class UTDDefaultAttributeInitComponent;
class UAttributeSet;
class UAbilitySystemComponent;
class UTDGameplayAbility;

/**
 * ATDCharacterBase
 *
 * Abstract base character class implementing IAbilitySystemInterface for GAS.
 * - Use as base for both player and AI characters that require Ability System integration.
 * - Owns references to Ability System Component (ASC) and Attribute Set.
 * - Provides initialization path and accessors for UI/controllers.
 */
UCLASS(Abstract)
class RPG_TOPDOWN_API ATDCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGASCoreCombatInterface
{
	GENERATED_BODY()

public:
	// ===== Construction & Lifecycle =====

	/** Sets default values for this character's properties. */
	ATDCharacterBase();

	// ===== IAbilitySystemInterface =====

	/** Returns the owned Ability System Component (ASC). Required by IAbilitySystemInterface. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ===== Project Convenience =====

	/** Returns the owned Attribute Set for this character (used for GAS stats). */
	virtual UAttributeSet* GetAttributeSet() const;

protected:
	// ===== Engine overrides =====
	virtual void BeginPlay() override;

	// ===== Initialization =====

	/**
	 * InitializeAbilityActorInfo
	 * Initializes all GAS-related references and connects them to the correct actors/components.
	 * Expected to be called after possession/state acquisition.
	 */
	virtual void InitializeAbilityActorInfo();

	/**
	 * GrantStartupAbilities
	 * Grants all abilities in the StartupAbilities array to the character's ASC.
	 * Should be called after InitializeAbilityActorInfo when ASC is ready.
	 */
	virtual void GrantStartupAbilities();

	/**
	 * TestActivateAbilities (Optional)
	 * Attempts to activate the first granted ability for testing purposes.
	 * Only works if abilities are granted and activatable without input.
	 */
	UFUNCTION(BlueprintCallable, Category="GAS|Testing", CallInEditor=true)
	virtual void TestActivateFirstAbility();

	// ===== Components & GAS References =====

	/** Skeletal mesh for the character's weapon. Exposed to the editor for assignment and setup. */
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	/** Ability System Component for this actor.
	  * For players, often owned by the PlayerState; for AI, by the Character.
	  */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** AttributeSet for this actor, containing all modifiable gameplay stats. */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/** Component responsible for applying default attribute GameplayEffects. */
	UPROPERTY(EditAnywhere, Category="GAS|Attributes")
	TObjectPtr<UTDDefaultAttributeInitComponent> DefaultAttributeInitComponent;

	/** Array of abilities to grant to this character on initialization. */
	UPROPERTY(EditAnywhere, Category="GAS|Abilities")
	TArray<TSubclassOf<UTDGameplayAbility>> StartupAbilities;

	/** Whether to automatically test-activate the first granted ability on initialization (for testing). */
	UPROPERTY(EditAnywhere, Category="GAS|Abilities")
	bool bAutoTestActivateFirstAbility = false;
};