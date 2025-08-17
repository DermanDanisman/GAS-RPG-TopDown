// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CoreAttributeSet.generated.h"

/**
 * Macro that creates standardized getter/setter/initter functions for attributes.
 * This significantly reduces boilerplate code when working with attributes.
 *
 * Example usage: ATTRIBUTE_ACCESSORS(UGASAttributeSet, Health) creates:
 * - GetHealthAttribute() - Returns the FGameplayAttribute object
 * - GetHealth() - Returns the CurrentValue
 * - SetHealth() - Sets the CurrentValue
 * - InitHealth() - Sets the BaseValue (and CurrentValue)
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Lightweight container describing source and target context for a GameplayEffect
 * during PostGameplayEffectExecute. Populated via SetEffectProperties().
 *
 * Note:
 * - We prefer TObjectPtr for UObject-derived pointers for GC awareness.
 * - We only cache safe, readily available references (no expensive lookups here).
 */
USTRUCT()
struct FCoreEffectContext
{
	GENERATED_BODY()
	
	/** Full GameplayEffect context for additional data like hit results, instigator, causer, etc. */
	FGameplayEffectContextHandle GameplayEffectContextHandle;

	/** Source (instigator) AbilitySystemComponent that applied the effect */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	/** Avatar actor of the source ASC (e.g., the character/pawn) */
	UPROPERTY()
	TObjectPtr<AActor> SourceAvatarActor = nullptr;
	
	/** Controller that owns/commands the source avatar (if any) */
	UPROPERTY()
	TObjectPtr<AController> SourceController = nullptr;

	/** Strongly-typed convenience access to the source character (if avatar is a character) */
	UPROPERTY()
	TObjectPtr<ACharacter> SourceCharacter = nullptr;

	/** Target (recipient) AbilitySystemComponent that received the effect */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetASC = nullptr;

	/** Avatar actor of the target ASC (e.g., the character/pawn) */
	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor = nullptr;
	
	/** Controller that owns/commands the target avatar (if any) */
	UPROPERTY()
	TObjectPtr<AController> TargetController = nullptr;

	/** Strongly-typed convenience access to the target character (if avatar is a character) */
	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter = nullptr;
};

/**
 * AttributeSet class for managing character attributes like Health, Mana, and Stamina.
 * 
 * Implements proper attribute clamping via:
 * - PreAttributeChange: Clamps CurrentValue changes (from any source, including Duration/Infinite GEs and direct sets)
 * - PreAttributeBaseChange: Clamps BaseValue changes (from Instant/Periodic GEs)
 * 
 * This dual-clamping approach prevents attributes from exceeding valid ranges
 * in both temporary and permanent modifications (avoids "invisible buffers").
 */
UCLASS()
class GASCORE_API UCoreAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UCoreAttributeSet();
	
	/**
	 * Registers attributes for replication with RepNotify. Using REPNOTIFY_Always ensures
	 * OnRep handlers fire even when values appear unchanged locally (important for prediction/UI).
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Called before CurrentValue is modified for any attribute.
	 * We clamp the incoming value to [0, Max] to keep values consistent for temporary changes.
	 *
	 * Affects temporary modifiers from Duration/Infinite GameplayEffects and direct sets.
	 *
	 * @param Attribute The attribute about to change
	 * @param NewValue  The requested new CurrentValue (clamped in-place)
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * Called before BaseValue is modified for any attribute.
	 * We clamp the incoming base to [0, Max] to prevent overflows from Instant/Periodic effects.
	 * Without this, BaseValue could exceed Max and create a non-visible buffer.
	 *
	 * Affects permanent modifiers from Instant/Periodic GameplayEffects.
	 * Prevents hidden overflow buffers (e.g., Health.BaseValue > MaxHealth).
	 *
	 * @param Attribute The attribute about to change
	 * @param NewValue  The requested new BaseValue (clamped in-place)
	 */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/**
	 * Called after a GameplayEffect has executed and applied its modifiers.
	 * Use this to react to instant changes (e.g., damage/heal resolution, adjusting current when max changes).
	 * We currently populate a context struct for convenient access to source/target data.
	 *
	 * Typical uses:
	 * - Apply final damage/healing to attributes
	 * - Adjust current values when max values change
	 * - Trigger gameplay cues, death checks, etc.
	 *
	 * Current implementation only gathers effect properties; extend as needed.
	 */
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	// =========================
	// Health Attributes
	// =========================

	/** Current health value - how much health the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Attributes|Vitals")
	FGameplayAttributeData Health;
	/** Generates attribute accessors (getter/setter/initter) for Health */
	ATTRIBUTE_ACCESSORS(UCoreAttributeSet, Health);

	/** Maximum health value - upper bound for Health */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Attributes|Vitals")
	FGameplayAttributeData MaxHealth;
	/** Generates attribute accessors for MaxHealth */
	ATTRIBUTE_ACCESSORS(UCoreAttributeSet, MaxHealth);

	// =========================
	// Mana Attributes
	// =========================

	/** Current mana value - resource for casting abilities */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="Attributes|Vitals")
	FGameplayAttributeData Mana;
	/** Generates attribute accessors for Mana */
	ATTRIBUTE_ACCESSORS(UCoreAttributeSet, Mana);
	
	/** Maximum mana value - upper bound for Mana */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="Attributes|Vitals")
	FGameplayAttributeData MaxMana;
	/** Generates attribute accessors for MaxMana */
	ATTRIBUTE_ACCESSORS(UCoreAttributeSet, MaxMana);

	// =========================
	// Stamina Attributes
	// =========================

	/** Current stamina value - resource for physical actions like sprinting */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Stamina, Category="Attributes|Vitals")
	FGameplayAttributeData Stamina;
	/** Generates attribute accessors for Stamina */
	ATTRIBUTE_ACCESSORS(UCoreAttributeSet, Stamina);

	/** Maximum stamina value - upper bound for Stamina */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxStamina, Category="Attributes|Vitals")
	FGameplayAttributeData MaxStamina;
	/** Generates attribute accessors for MaxStamina */
	ATTRIBUTE_ACCESSORS(UCoreAttributeSet, MaxStamina);

	// =======================================
	// Health Attributes Rep Notify Functions
	// =======================================
	
	/**
	 * RepNotify for Health attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	/** RepNotify for MaxHealth attribute */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	// =======================================
	// Mana Attributes Rep Notify Functions
	// =======================================
	
	/** RepNotify for Mana attribute */
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	/** RepNotify for MaxMana attribute */
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	// =======================================
	// Stamina Attributes Rep Notify Functions
	// =======================================
	
	/** RepNotify for Stamina attribute */
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;
	
	/** RepNotify for MaxStamina attribute */
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;

private:
	/**
	 * Helper that extracts and caches commonly needed source/target data from the
	 * GameplayEffect callback. Keeps PostGameplayEffectExecute clean and focused.
	 */
	void PopulateEffectContext(const struct FGameplayEffectModCallbackData& Data, FCoreEffectContext& InEffectContext) const;
};
