// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// File: GASCoreAttributeSet.h (header)
// Purpose:
//   - Provide a small, reusable base AttributeSet for GAS (Gameplay Ability System).
//   - Centralize common behaviors: Current↔Max clamping, rounding policy, and effect-context extraction.
//   - Keep the plugin free of game-specific attributes; games derive and declare only the attributes they need.
//
// Why:
//   - GAS attributes must be declared statically on UAttributeSet classes; you can’t “add attributes at runtime”.
//   - Projects often duplicate clamp and “current <= max” logic across multiple sets.
//   - This base allows games to register attribute pairs once and get consistent clamping + rounding behavior.
//
// Usage (recommended pattern):
//   1) Derive your game AttributeSet from UGASCoreAttributeSet.
//   2) Declare your attributes (UPROPERTY FGameplayAttributeData …).
//   3) In your derived class constructor, call RegisterCurrentMaxPair for each Current↔Max pair (e.g., Health↔MaxHealth).
//   4) Optionally override GetRoundingDecimals to use different precision per attribute (e.g., 0 for vitals, 2 for regen).
//   5) Let the base handle PreAttributeChange, PreAttributeBaseChange, and PostGameplayEffectExecute clamping and rounding.
//
// Notes on clamping callbacks:
//   - PreAttributeChange: fires for CurrentValue changes (Duration/Infinite GEs, direct sets, aggregator updates).
//   - PreAttributeBaseChange: fires for BaseValue changes (Instant/Periodic GEs).
//   - PostGameplayEffectExecute: final server-authoritative step; safe place to SetX() and finalize clamping
//     (especially relevant when Max values change and Current must be re-clamped).
//
// Rounding policy:
//   - By default, all attribute writes (Current and Base) are rounded to DefaultRoundingDecimals (default 0 → integers).
//   - Override GetRoundingDecimals to customize per attribute.
//
// Effect context helper:
//   - FGASCoreEffectContext is a lightweight, GC-safe container of source/target references available in GE callbacks.
//   - Use PopulateCoreEffectContext in PostGameplayEffectExecute (or elsewhere) to quickly gather safe pointers.
//
// Replication:
//   - This base does not declare any attributes; derived sets should handle DOREPLIFETIME + RepNotify as needed.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GASCoreAttributeSet.generated.h"

// Forward declarations (keep header lean)
class UAbilitySystemComponent;
class AActor;
class AController;
class ACharacter;
struct FGameplayEffectModCallbackData;

// Same ATTRIBUTE_ACCESSORS macro you already use
// - Defines a static FGameplayAttribute getter (Get<PropertyName>Attribute())
// - Defines value getters/setters/initters for FGameplayAttributeData property
// These accessors are essential for our Tag→Accessor registry because we store
// function pointers to the static FGameplayAttribute getters.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Lightweight container describing source and target context for a GameplayEffect
 * during PostGameplayEffectExecute (or any GE callback).
 *
 * Intent:
 * - Provide safe, readily available references (ASC, avatar, controller, character) for both Source and Target.
 * - Avoid expensive lookups in hot paths; only cache what we already have access to via the GE callback.
 *
 * GC-safety:
 * - UObject-derived pointers use TObjectPtr to keep GC aware.
 */
USTRUCT()
struct FGASCoreEffectContext
{
	GENERATED_BODY()
	
	/** Full GE context for additional data (hit results, instigator, causer, etc.). */
	FGameplayEffectContextHandle GameplayEffectContextHandle;

	/** Source (instigator) AbilitySystemComponent that applied the effect. */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	/** Avatar actor of the source ASC (e.g., character or pawn). */
	UPROPERTY()
	TObjectPtr<AActor> SourceAvatarActor = nullptr;
	
	/** Controller that owns/commands the source avatar (player or AI). */
	UPROPERTY()
	TObjectPtr<AController> SourceController = nullptr;

	/** Strongly-typed convenience pointer to the source character (if avatar is a character). */
	UPROPERTY()
	TObjectPtr<ACharacter> SourceCharacter = nullptr;

	/** Target (recipient) AbilitySystemComponent that received the effect. */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetASC = nullptr;

	/** Avatar actor of the target ASC (e.g., character or pawn). */
	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor = nullptr;
	
	/** Controller that owns/commands the target avatar (player or AI). */
	UPROPERTY()
	TObjectPtr<AController> TargetController = nullptr;

	/** Strongly-typed convenience pointer to the target character (if avatar is a character). */
	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter = nullptr;
};

/**
 * UGASCoreAttributeSet
 *
 * Abstract base AttributeSet that provides:
 * - Registration of Current↔Max attribute pairs (e.g., Health↔MaxHealth).
 * - Automatic clamping so Current ∈ [0, Max] in PreAttributeChange and PreAttributeBaseChange.
 * - Final authoritative clamping in PostGameplayEffectExecute when Max changes.
 * - A rounding policy applied consistently to both Current and Base values.
 * - A helper to populate a shared effect context for convenience in callbacks.
 * - A Tag→AttributeAccessor registry to support generic UI broadcasting.
 *
 * Design:
 * - This class does not declare any attributes. Your game derives and declares attributes it needs.
 * - Derived classes typically do replication (GetLifetimeReplicatedProps + RepNotify).
 */
UCLASS(Abstract)
class GASCORE_API UGASCoreAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGASCoreAttributeSet();

protected:
	/**
	 * Register a Current ↔ Max attribute pair for automatic clamping.
	 * Call from the derived-class constructor after attributes are defined.
	 *
	 * Example:
	 *   RegisterCurrentMaxPair(GetHealthAttribute(), GetMaxHealthAttribute());
	 */
	void RegisterCurrentMaxPair(const FGameplayAttribute& Current, const FGameplayAttribute& Max);

	// ----------------------
	// UAttributeSet overrides
	// ----------------------

	/**
	 * PreAttributeChange
	 * - Fires before the CurrentValue of an attribute changes (Duration/Infinite GEs, direct sets, aggregator updates).
	 * - We clamp Current to [0, Max] and apply rounding.
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * PreAttributeBaseChange
	 * - Fires before the BaseValue of an attribute changes (Instant/Periodic GEs).
	 * - We clamp Base to [0, Max(Current)] and apply rounding.
	 */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/**
	 * PostGameplayEffectExecute
	 * - Fires after an Instant/Periodic GameplayEffect executes (server-authoritative).
	 * - If a Max attribute changed, we re-clamp its paired Current and apply rounding.
	 * - Safe place to call SetX() to modify attributes directly.
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ----------------------
	// Optional hooks
	// ----------------------

	/** Called when a Current value was clamped by its Max in PreAttributeChange. */
	virtual void OnCurrentClampedByMax(const FGameplayAttribute& CurrentAttr, const FGameplayAttribute& MaxAttr, float OldCurrent, float NewCurrent) {}

	/** Called when a Max changed and the corresponding Current had to be clamped in PostGameplayEffectExecute. */
	virtual void OnMaxAttributeChangedAndClamped(const FGameplayAttribute& CurrentAttr, const FGameplayAttribute& MaxAttr, float OldCurrent, float NewCurrent) {}

	// ----------------------
	// Rounding policy
	// ----------------------

	/**
	 * Default number of decimals for rounding attribute values.
	 * - 0 = integers (typical for Health/Mana/Stamina).
	 * - Override GetRoundingDecimals to return different precision per attribute.
	 */
	UPROPERTY(EditDefaultsOnly, Category="GAS|Attributes")
	int32 DefaultRoundingDecimals = 0;

	/**
	 * Return the number of decimals to keep for a specific attribute.
	 * Override to provide per-attribute precision (e.g., 2 for regeneration).
	 */
	virtual int32 GetRoundingDecimals(const FGameplayAttribute& Attribute) const { return DefaultRoundingDecimals; }

	/**
	 * Round a value to N decimals (half away from zero).
	 * Applied after clamping and on direct writes (SetCurrentNumeric).
	 */
	static float RoundToDecimals(float Value, int32 Decimals);

	// ----------------------
	// Utilities
	// ----------------------

	/** Read the CurrentValue of an attribute belonging to this set. */
	float GetCurrentNumeric(const FGameplayAttribute& Attr) const;

	/** Read the BaseValue of an attribute belonging to this set. */
	float GetBaseNumeric(const FGameplayAttribute& Attr) const;

	/**
	 * Set the Current/Base value (via ASC) for an attribute belonging to this set.
	 * - Applies rounding before writing.
	 * - Uses SetNumericAttributeBase to assign the base (authoritative) value.
	 */
	void SetCurrentNumeric(const FGameplayAttribute& Attr, float NewValue);

	/**
	 * Populate a lightweight effect context with source/target references available from the GE callback.
	 * Safe to call in PostGameplayEffectExecute and similar callbacks.
	 */
	void PopulateCoreEffectContext(const FGameplayEffectModCallbackData& Data,
							       FGASCoreEffectContext& InEffectContext) const;

private:
	
	// Fast lookup maps for Current <-> Max pairs; populated by RegisterCurrentMaxPair.
	TMap<FGameplayAttribute, FGameplayAttribute> CurrentToMax;
	TMap<FGameplayAttribute, FGameplayAttribute> MaxToCurrent;

	/** Find the Max attribute paired with a Current attribute. */
	bool TryGetMaxForCurrent(const FGameplayAttribute& Current, FGameplayAttribute& OutMax) const;

	/** Find the Current attribute paired with a Max attribute. */
	bool TryGetCurrentForMax(const FGameplayAttribute& Max, FGameplayAttribute& OutCurrent) const;

	/** Locate the underlying FGameplayAttributeData (const). */
	static const FGameplayAttributeData* FindAttributeDataConst(const FGameplayAttribute& Attr, const UAttributeSet* Set);

	/** Locate the underlying FGameplayAttributeData (mutable). */
	static FGameplayAttributeData* FindAttributeData(const FGameplayAttribute& Attr, UAttributeSet* Set);
};