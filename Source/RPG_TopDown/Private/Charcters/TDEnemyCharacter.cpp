// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Charcters/TDEnemyCharacter.h"

#include "HighlightActor.h"
#include "GASCore/Public/Attributes/CoreAttributeSet.h"
#include "GASCore/Public/Components/CoreAbilitySystemComponent.h"
#include "Components/TDAbilitySystemComponent.h"
#include "RPG_TopDown/RPG_TopDown.h"

// Sets default values
ATDEnemyCharacter::ATDEnemyCharacter()
{
	// Set mesh collision to respond to the custom highlight channel.
	GetMesh()->SetCollisionResponseToChannel(HIGHLIGHTABLE, ECR_Block);

	// Create the Ability System Component for the AI enemy.
	// Unlike player characters, AI own their own ASC and AttributeSet.
	AbilitySystemComponent = CreateDefaultSubobject<UTDAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); // Enable replication for multiplayer.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal); // Minimal: fewer network updates.

	// Create the Attribute Set for this AI character.
	AttributeSet = CreateDefaultSubobject<UCoreAttributeSet>("AttributeSet");
}

// Called when the game starts or when spawned
void ATDEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the Ability System with this actor as both owner and avatar.
	// For AI, this is done here since AI owns its own ASC/AttributeSet.
	InitializeAbilityActorInfo();
}

void ATDEnemyCharacter::InitializeAbilityActorInfo()
{
	// --- AI-controlled character setup ---
	// AI characters own their own ASC and AttributeSet.
	if (AbilitySystemComponent)
	{
		// Initialize the ASC's actor info, using this character as both owner and avatar.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// Bind ASC delegates (e.g., attribute change broadcasts) for this component type.
		Cast<UTDAbilitySystemComponent>(AbilitySystemComponent)->BindASCDelegates();
	}
}

void ATDEnemyCharacter::HighlightActor()
{
	// Mark as highlighted for internal logic or UI.
	bHighlighted = true;

	// Enable custom depth rendering for outline/FX.
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	// Also highlight the weapon mesh (if present).
	WeaponMesh->SetRenderCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void ATDEnemyCharacter::UnHighlightActor()
{
	// Unmark as highlighted.
	bHighlighted = false;

	// Disable custom depth rendering.
	GetMesh()->SetRenderCustomDepth(false);
	WeaponMesh->SetRenderCustomDepth(false);
}

int32 ATDEnemyCharacter::GetActorLevel()
{
	// AI enemies keep their level on the character itself.
	return EnemyCharacterLevel;
}