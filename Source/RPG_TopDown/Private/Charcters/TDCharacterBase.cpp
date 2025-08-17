// Fill out your copyright notice in the Description page of Project Settings.


#include "Charcters/TDCharacterBase.h"


ATDCharacterBase::ATDCharacterBase()
{
	// Disable tick by default for performance. Subclasses can enable as needed.
    PrimaryActorTick.bCanEverTick = false;

    // Create the skeletal mesh component for the weapon.
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
    // Attach the weapon mesh to the main character mesh at the "WeaponHandSocket" socket.
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

