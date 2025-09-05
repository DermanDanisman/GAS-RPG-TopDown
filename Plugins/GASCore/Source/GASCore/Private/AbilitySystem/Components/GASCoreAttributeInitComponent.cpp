// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "GASCore/Public/AbilitySystem/Components/GASCoreAttributeInitComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UGASCoreAttributeInitComponent::UGASCoreAttributeInitComponent()
{
	// This is a fire-and-forget initialization helper; no need to tick every frame.
	PrimaryComponentTick.bCanEverTick = false;
}

void UGASCoreAttributeInitComponent::InitializeDefaultAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent) const
{
	// Apply initial attributes in dependency order:
	// 1) Primary (base stats) -> 2) Secondary (derived, often MMC-based) -> 3) Vital (set current = max).
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f, TargetAbilitySystemComponent);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f, TargetAbilitySystemComponent);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f, TargetAbilitySystemComponent);
}

void UGASCoreAttributeInitComponent::ApplyEffectToSelf(
	TSubclassOf<UGameplayEffect> GameplayEffectClass,
	float Level,
	UAbilitySystemComponent* TargetAbilitySystemComponent) const
{
	// Development-time checks to surface incorrect usage early.
	// - TargetAbilitySystemComponent must be valid.
	// - GameplayEffectClass (GE class) must be assigned in BP or defaults.
	check(IsValid(TargetAbilitySystemComponent));
	check(GameplayEffectClass);

	// Build an effect context from the target ASC.
	// Consider adding SourceObject/Instigator if needed (e.g., for MMCs reading interfaces).
	FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();

	// Provide a SourceObject for downstream systems (e.g., MMC querying ICombatInterface).
	// Here, we use the owning Actor of this component. Change if attribution should differ.
	EffectContextHandle.AddSourceObject(GetOwner());

	// Create an outgoing spec for the initialization GE at the given Level.
	// Level can drive scalable floats or SetByCaller magnitudes inside the GE.
	const FGameplayEffectSpecHandle EffectSpec =
		TargetAbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);

	// Bail out safely if the spec could not be created (e.g., class not set).
	if (!EffectSpec.IsValid() || !EffectSpec.Data.IsValid())
	{
		return;
	}

	// Apply the spec "to target" where the target is the same ASC (self-application).
	// Networking:
	// - Prefer calling on the server so attribute replication updates clients authoritatively.
	TargetAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), TargetAbilitySystemComponent);
}