// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "TDPlayerState.generated.h"

class UAttributeSet;

/**
 * ATDPlayerState
 *
 * Custom PlayerState for the GAS TopDown RPG project.
 * - Implements IAbilitySystemInterface to provide access to GAS components.
 * - Owns the authoritative AbilitySystemComponent and AttributeSet for player-controlled characters.
 *
 * In Unreal GAS, PlayerState is the preferred owner for AbilitySystemComponent (ASC) and attributes for persistent, replicated state.
 */
UCLASS()
class RPG_TOPDOWN_API ATDPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ATDPlayerState();

	/** Returns the owned AbilitySystemComponent for GAS integration. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	/** Returns the owned AttributeSet for this player (for GAS attributes). */
	virtual UAttributeSet* GetAttributeSet() const;

protected:

	virtual void BeginPlay() override;
	
	/** The AbilitySystemComponent for this player, authoritatively owned and replicated. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** The AttributeSet for this player, contains all replicated gameplay attributes. */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};
