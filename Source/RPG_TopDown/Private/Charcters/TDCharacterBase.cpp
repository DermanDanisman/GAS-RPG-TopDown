// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Charcters/TDCharacterBase.h"

ATDCharacterBase::ATDCharacterBase()
{
	// Disable tick by default for performance. Subclasses can enable as needed.
    PrimaryActorTick.bCanEverTick = false;

	/*DefaultAttributeInitComponent = CreateDefaultSubobject<UTDDefaultAttributeInitComponent>("DefaultAttributeInitComponent");*/

    // Create the skeletal mesh component for the weapon.
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
    // Attach the weapon mesh to the main character mesh at the "WeaponHandSocket" sockeSt.
    WeaponMesh->SetupAttachment(GetMesh(), "WeaponHandSocket");
    // Disable collision on the weapon mesh (can be enabled in subclasses if needed).
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATDCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATDCharacterBase::InitializeAbilityActorInfo()
{
}

UAbilitySystemComponent* ATDCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* ATDCharacterBase::GetAttributeSet() const
{
	return AttributeSet;
}

