// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CorePrimaryAttributeInitComponent.h"

#include "AbilitySystemComponent.h"


UCorePrimaryAttributeInitComponent::UCorePrimaryAttributeInitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCorePrimaryAttributeInitComponent::InitializePrimaryAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent) const
{
	check(IsValid(TargetAbilitySystemComponent));
	check(DefaultPrimaryAttributes);
	
	const FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	const FGameplayEffectSpecHandle EffectSpec = TargetAbilitySystemComponent->MakeOutgoingSpec(DefaultPrimaryAttributes, 1.f, EffectContextHandle);
	TargetAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), TargetAbilitySystemComponent);
}
