// Fill out your copyright notice in the Description page of Project Settings.


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
