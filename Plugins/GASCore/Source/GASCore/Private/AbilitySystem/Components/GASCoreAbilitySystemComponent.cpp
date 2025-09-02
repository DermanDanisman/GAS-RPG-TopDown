// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Implementation notes:
// - We bind using AddUObject (vs AddLambda) so GC/unbinding is managed by UE
// - Call BindASCDelegates() once after InitAbilityActorInfo
// - If you grant/remove abilities repeatedly or re-init actor info (e.g., on possession changes),
//   ensure you don't bind multiple times (track a bool or remove binding if needed).

#include "GASCore/Public/AbilitySystem/Components/GASCoreAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/GASCoreGameplayAbility.h"

void UGASCoreAbilitySystemComponent::BindASCDelegates()
{
	// Register to receive a callback whenever a GameplayEffect is applied to self.
	// Using AddUObject ties the delegate lifetime to this UObject (safe unbinding on destruction).
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UGASCoreAbilitySystemComponent::HandleGameplayEffectAppliedToSelf);
}

void UGASCoreAbilitySystemComponent::HandleGameplayEffectAppliedToSelf(
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec,
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	// Gather asset tags from the applied spec.
	// Note: GetAllAssetTags aggregates inheritable owned tags, granted tags, and any tags added to the spec at runtime.
	FGameplayTagContainer GameplayTagContainer;
	GameplayEffectSpec.GetAllAssetTags(GameplayTagContainer);

	// Forward to consumers (e.g., HUD Widget Controller) for UI reactions.
	OnEffectAssetTags.Broadcast(GameplayTagContainer);

	// Optional: You could also surface GrantedTags or Cue tags if needed:
	// GameplayEffectSpec.CapturedSourceTags.GetSpecTags();
	// or GameplayEffectSpec.Def->InheritableGameplayEffectTags.CombinedTags;
}

void UGASCoreAbilitySystemComponent::AddCharacterAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& InStartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : InStartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UGASCoreGameplayAbility* CoreGameplayAbility = Cast<UGASCoreGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(CoreGameplayAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void UGASCoreAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		FGameplayTagContainer& DynamicSpecSourceTags = AbilitySpec.GetDynamicSpecSourceTags();
		if (DynamicSpecSourceTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UGASCoreAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		FGameplayTagContainer& DynamicSpecSourceTags = AbilitySpec.GetDynamicSpecSourceTags();
		if (DynamicSpecSourceTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}
