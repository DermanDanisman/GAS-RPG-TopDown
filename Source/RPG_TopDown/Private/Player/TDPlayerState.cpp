// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Player/TDPlayerState.h"

#include "Attributes/CoreAttributeSet.h"
#include "Components/CoreAbilitySystemComponent.h"

ATDPlayerState::ATDPlayerState()
{
	// Increase net update frequency for responsive attribute and state replication.
	SetNetUpdateFrequency(100.f);

	// Create the GAS AbilitySystemComponent as a default subobject.
	// PlayerState is the authoritative owner for player-controlled characters.
	AbilitySystemComponent = CreateDefaultSubobject<UCoreAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); // Enable replication for multiplayer.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the GAS AttributeSet as a default subobject.
	AttributeSet = CreateDefaultSubobject<UCoreAttributeSet>("AttributeSet");
}

void ATDPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

UAbilitySystemComponent* ATDPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* ATDPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
