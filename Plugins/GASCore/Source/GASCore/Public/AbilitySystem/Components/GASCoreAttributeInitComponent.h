// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

// CorePrimaryAttributeInitComponent
// Purpose:
// - Minimal, plug-and-play ActorComponent that applies an "Initial Primary Attributes"
//   Gameplay Effect (GE) to a target Ability System Component (ASC).
// - Designed so game projects can add this component to their Character (or other actors)
//   and call InitializePrimaryAttributes() right after ASC->InitAbilityActorInfo(...).
//
// Usage (recommended call site):
// - After you initialize the ASC's ActorInfo (Owner/Avatar are valid), call:
//     InitComp->InitializePrimaryAttributes(AbilitySystemComponent);
// - In multiplayer, prefer calling this on the server only (HasAuthority()) and let attribute
//   replication update clients.
//
// Notes:
// - This component does not resolve an ASC automatically. You must pass a valid ASC pointer.
//   If you want auto-resolution (e.g., via IAbilitySystemInterface on the owner or its PlayerState),
//   extend this component accordingly.
// - The GE asset referenced by DefaultPrimaryAttributes should be:
//     * Instant policy
//     * One Override modifier per primary attribute (e.g., STR/DEX/INT/END/VIG)
//     * Magnitudes set to your initial values
//
// Implementation detail:
// - We create an outgoing spec on the provided ASC and apply it "to target" where the target is
//   the same ASC (equivalent to ApplyToSelf, but using the ApplyGameplayEffectSpecToTarget API).
//
// File layout note:
// - Header-style declarations appear above and the implementation below in the same file for brevity.
//   In production, place the UCLASS in a .h file and function bodies in a .cpp file.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GASCoreAttributeInitComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * UCorePrimaryAttributeInitComponent
 *
 * Lightweight component that applies a single, project-authored GameplayEffect to initialize
 * primary attributes. Keep the GE asset in the game project (not the plugin) so values remain
 * game-specific.
 *
 * Teachable points:
 * - Apply after InitAbilityActorInfo so the ASC has a valid ActorInfo (Owner/Avatar).
 * - Prefer server-authoritative application; clients receive replicated attribute values.
 * - Use Override modifiers in the init GE to make starting values explicit.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASCORE_API UGASCoreAttributeInitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties.
	// We disable ticking; this component performs a one-shot initialization.
	UGASCoreAttributeInitComponent();

	/**
	 * Gameplay Effect that sets initial primary attributes (e.g., STR/DEX/INT/END/VIG).
	 *
	 * Expected asset configuration:
	 * - Duration Policy: Instant (applies once)
	 * - Modifiers: One Override per primary attribute you want to initialize
	 * - Magnitudes: Your initial values (e.g., STR=10, DEX=12, INT=14, END=8, VIG=9)
	 *
	 * Author the asset in the game project. Do not hard-code defaults in the plugin.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GASCore|Attribute Init Component|Init")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	// Infinite GE that computes secondary/derived attributes (e.g., MaxHealth/MaxMana via MMCs).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GASCore|Attribute Init Component|Init")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	// Instant GE that sets current Health/Mana equal to their Max after secondaries are ready.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GASCore|Attribute Init Component|Init")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;


	/**
	 * Initialize primary, secondary, and vital attributes by applying configured GEs
	 * to the provided ASC in the correct order.
	 *
	 * Parameters:
	 * - TargetAbilitySystemComponent: The ASC that should receive the init GEs.
	 *   Must be valid. This function does not auto-resolve an ASC if null.
	 *
	 * When to call:
	 * - After ASC->InitAbilityActorInfo(Owner, Avatar) so ActorInfo is valid.
	 *
	 * Networking guidance:
	 * - Recommended to call on the server only (HasAuthority()) and rely on replication.
	 *   If you choose to call on both server and client, ensure values are identical to
	 *   avoid visual discrepancies before replication.
	 *
	 * Safety:
	 * - Uses check() to catch misuse during development (null ASC or missing GE).
	 *   Consider replacing with guards/ensure in shipping builds to avoid crashing.
	 */
	UFUNCTION(BlueprintCallable, Category="GASCore|Attribute Init Component|Init")
	virtual void InitializeDefaultAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent) const;
	
	/**
	 * Helper to build an effect spec and apply it to the same ASC (self-application).
	 * - Level parameter enables scalable values or SetByCaller scaling in the GE.
	 */
	virtual void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, UAbilitySystemComponent* TargetAbilitySystemComponent) const;
};