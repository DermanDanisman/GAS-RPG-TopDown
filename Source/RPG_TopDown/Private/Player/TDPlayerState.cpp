// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Player/TDPlayerState.h"

#include "GASCore/Public/Attributes/CoreAttributeSet.h"
#include "Components/TDAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

ATDPlayerState::ATDPlayerState()
{
	// Increase net update frequency for responsive attribute and state replication.
	SetNetUpdateFrequency(100.f);

	// Create the GAS AbilitySystemComponent as a default subobject.
	// PlayerState is the authoritative owner for player-controlled characters.
	AbilitySystemComponent = CreateDefaultSubobject<UTDAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); // Enable replication for multiplayer.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); // Server sends important GE data.

	// Create the GAS AttributeSet as a default subobject.
	AttributeSet = CreateDefaultSubobject<UCoreAttributeSet>("AttributeSet");
}

void ATDPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register PlayerLevel for replication so clients receive updates.
	DOREPLIFETIME(ATDPlayerState, PlayerLevel);
}

void ATDPlayerState::BeginPlay()
{
	Super::BeginPlay();
	// PlayerState exists on server and owning client early; GAS may initialize later via Character.
}

UAbilitySystemComponent* ATDPlayerState::GetAbilitySystemComponent() const
{
	// IAbilitySystemInterface implementation.
	return AbilitySystemComponent;
}

UAttributeSet* ATDPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void ATDPlayerState::OnRep_PlayerLevel(int32 OldLevel)
{
	// Client-side reaction to level changes (e.g., update HUD, re-run derived stat display).
	// Note: Reapplying derived infinite GE (if required) should be initiated on server.
}