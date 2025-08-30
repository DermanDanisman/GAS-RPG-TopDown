// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "AbilitySystem/Components/TDDefaultAttributeInitComponent.h"

UTDDefaultAttributeInitComponent::UTDDefaultAttributeInitComponent()
{
	// Tick is enabled by default to allow future runtime checks or re-application logic if desired.
	// If unused, you may disable ticking for performance.
	PrimaryComponentTick.bCanEverTick = true;
}

void UTDDefaultAttributeInitComponent::InitializeDefaultAttributes(
	UAbilitySystemComponent* TargetAbilitySystemComponent) const
{
	// Defer to base for canonical application (reads configured GameplayEffects/DataAssets).
	Super::InitializeDefaultAttributes(TargetAbilitySystemComponent);

	// Project hook:
	// - Apply additional context-sensitive effects (class, level, equipment).
	// - Run validation and logging.
}