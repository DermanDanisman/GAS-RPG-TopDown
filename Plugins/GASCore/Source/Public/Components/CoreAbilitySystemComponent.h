// Copyright:
// © 2025 Heathrow (Derman). All rights reserved.
//
// Purpose:
// - Light wrapper over UAbilitySystemComponent that exposes a simple multicast delegate
//   to forward GameplayEffect "asset tags" applied to this ASC to interested systems (e.g., UI).
//
// Why:
// - GAS GameplayEffects can carry asset tags that describe their semantics (e.g., UI.Message.HealthPotion).
// - Surfacing those tags at the moment of application allows the UI to react (toast messages, icons, sounds)
//   without coupling UI logic to effect classes/assets.
//
// Usage:
// - After initializing ASC actor info (InitAbilityActorInfo), call BindASCDelegates() once to register the hook
// - Bind to OnEffectAssetTags to receive FGameplayTagContainer whenever a GE is applied to self
//
// Replication notes:
// - The delegate fires on the instance where the application occurs. In a typical MP setup, UI belongs to
//   the owning client; ensure you fire on or route to the owning client as appropriate if needed.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CoreAbilitySystemComponent.generated.h"

/** Multicast delegate that carries GameplayEffect asset tags gathered from the applied spec. */
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTagsSignature, const FGameplayTagContainer& /*AssetTags*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASCORE_API UCoreAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	/**
	 * Registers this ASC to receive callbacks when GameplayEffects are applied to self.
	 * Call this once after InitAbilityActorInfo(Owner, Avatar), when the ASC is fully initialized.
	 *
	 * Implementation detail:
	 * - Binds HandleGameplayEffectAppliedToSelf to OnGameplayEffectAppliedDelegateToSelf via AddUObject
	 * - AddUObject ensures safe unbinding if this component is GC'd
	 */
	void BindASCDelegates();

	/**
	 * Fires whenever a GameplayEffect is applied to this ASC (self), providing the asset tag container
	 * extracted from the effect spec. Consumers can filter by tag families (e.g., "UI.Message").
	 */
	FEffectAssetTagsSignature OnEffectAssetTags;

protected:

	/**
	 * Internal handler for GameplayEffect application to self.
	 * - Gathers all asset tags from the incoming GameplayEffectSpec
	 * - Broadcasts them via OnEffectAssetTags
	 *
	 * @param AbilitySystemComponent The ASC receiving the effect (i.e., this)
	 * @param GameplayEffectSpec     The spec for the effect being applied
	 * @param ActiveGameplayEffectHandle Handle to the active GE instance (if any)
	 */
	void HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
};