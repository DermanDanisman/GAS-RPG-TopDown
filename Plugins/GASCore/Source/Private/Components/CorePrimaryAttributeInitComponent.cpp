// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Components/CorePrimaryAttributeInitComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"


UCorePrimaryAttributeInitComponent::UCorePrimaryAttributeInitComponent()
{
	// This is a fire-and-forget initialization helper; no need to tick.
	PrimaryComponentTick.bCanEverTick = false;
}

void UCorePrimaryAttributeInitComponent::InitializePrimaryAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent) const
{
	// Development-time checks to surface incorrect usage early.
	// - TargetAbilitySystemComponent must be valid.
	// - DefaultPrimaryAttributes (GE class) must be assigned in BP or defaults.
	check(IsValid(TargetAbilitySystemComponent));
	check(DefaultPrimaryAttributes);

	// Build an effect context from the target ASC.
	// You may optionally add metadata here,
	// such as SourceObject or Instigator, if you need it later for attribution.
	const FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();

	// Create an outgoing spec for the initialization GE at level 1.0.
	// If you need level scaling, pass a different level or set SetByCaller magnitudes inside the spec.
	const FGameplayEffectSpecHandle EffectSpec =
		TargetAbilitySystemComponent->MakeOutgoingSpec(DefaultPrimaryAttributes, 1.f, EffectContextHandle);

	// Apply the spec "to target" where the target is the same ASC (self-application).
	// Note on pointer handling:
	// - EffectSpec.Data is a TSharedPtr<FGameplayEffectSpec>.
	// - Data.Get() yields a raw pointer; we then dereference (*) because Apply expects a const ref.
	// Networking note:
	// - Prefer calling this on the server.
	// Attribute replication will update clients.
	TargetAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), TargetAbilitySystemComponent);
}
