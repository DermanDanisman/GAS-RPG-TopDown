// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Charcters/TDCharacterBase.h" // NOTE: Path as per project include; ensure module path is correct.

#include "AbilitySystem/Components/TDDefaultAttributeInitComponent.h"
#include "AbilitySystem/Abilities/TDGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"

ATDCharacterBase::ATDCharacterBase()
{
	// Disable tick by default for performance. Subclasses can enable as needed.
	PrimaryActorTick.bCanEverTick = false;

	// Optionally create default attribute init component in derived classes or via BP.
	DefaultAttributeInitComponent = CreateDefaultSubobject<UTDDefaultAttributeInitComponent>("DefaultAttributeInitComponent");

	// Create the skeletal mesh component for the weapon and attach to character mesh.
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponHandSocket"));

	// Disable collision on the weapon mesh (enable in subclasses if you need weapon traces).
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATDCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Post-begin initialization for GAS can be placed here or in possession/OnRep_PlayerState for players.
}

void ATDCharacterBase::InitializeAbilityActorInfo()
{
	// Intended for subclasses:
	// - Resolve owning ASC (PlayerState vs Character)
	// - Call ASC->InitAbilityActorInfo(OwnerActor, AvatarActor)
	// - Apply default attributes (DefaultAttributeInitComponent->InitializeDefaultAttributes)
	// - Grant startup abilities
}

void ATDCharacterBase::GrantStartupAbilities()
{
	// Only grant abilities on the server (authority required for ability granting)
	if (!HasAuthority())
	{
		return;
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	// Grant each ability in the StartupAbilities array
	for (const TSubclassOf<UTDGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (AbilityClass)
		{
			// Grant the ability to the ASC
			// The ability level defaults to 1, and InputID defaults to INDEX_NONE (no input binding)
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

UAbilitySystemComponent* ATDCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* ATDCharacterBase::GetAttributeSet() const
{
	return AttributeSet;
}