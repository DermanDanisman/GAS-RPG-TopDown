// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Charcters/TDCharacterBase.h" // NOTE: Path as per project include; ensure module path is correct.

#include "AbilitySystem/Components/TDDefaultAttributeInitComponent.h"
#include "AbilitySystem/Abilities/TDGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

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
			// Validate that this is actually a TDGameplayAbility subclass
			if (!AbilityClass->IsChildOf<UTDGameplayAbility>())
			{
				UE_LOG(LogTemp, Warning, TEXT("GrantStartupAbilities: %s is not a valid TDGameplayAbility subclass, skipping"), *AbilityClass->GetName());
				continue;
			}

			// Grant the ability to the ASC
			// The ability level defaults to 1, and InputID defaults to INDEX_NONE (no input binding)
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
			
			UE_LOG(LogTemp, Log, TEXT("GrantStartupAbilities: Granted ability %s to %s"), *AbilityClass->GetName(), *GetName());
		}
	}

	// Optionally test-activate the first ability for debugging/testing
	if (bAutoTestActivateFirstAbility && StartupAbilities.Num() > 0)
	{
		// Small delay to ensure abilities are fully granted before testing
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			TestActivateFirstAbility();
		});
	}
}

void ATDCharacterBase::TestActivateFirstAbility()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestActivateFirstAbility: No AbilitySystemComponent found"));
		return;
	}

	// Get all granted abilities
	TArray<FGameplayAbilitySpec*> ActivatableAbilities;
	AbilitySystemComponent->GetActivatableAbilities(ActivatableAbilities);

	if (ActivatableAbilities.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestActivateFirstAbility: No abilities granted to test"));
		return;
	}

	// Try to activate the first ability
	FGameplayAbilitySpec* FirstAbility = ActivatableAbilities[0];
	if (FirstAbility && FirstAbility->Ability)
	{
		UE_LOG(LogTemp, Log, TEXT("TestActivateFirstAbility: Attempting to activate %s"), *FirstAbility->Ability->GetName());
		
		// Try to activate the ability
		bool bActivated = AbilitySystemComponent->TryActivateAbility(FirstAbility->Handle);
		
		if (bActivated)
		{
			UE_LOG(LogTemp, Log, TEXT("TestActivateFirstAbility: Successfully activated ability"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TestActivateFirstAbility: Failed to activate ability (may require specific conditions)"));
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