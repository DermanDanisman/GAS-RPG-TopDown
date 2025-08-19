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
 * ATTRIBUTE CLAMPING STRATEGY:
 * This class implements a comprehensive three-tier clamping approach to prevent attribute overflow:
 * 
 * 1. PreAttributeChange():
 *    - WHEN: Always fires before CurrentValue modification (any source: Duration/Infinite GEs, direct sets, aggregator updates)
 *    - PURPOSE: Clamps visible CurrentValue to [0, MaxX] for consistency across all change sources
 *    - NOTE: Clamping NewValue here doesn't persist to underlying aggregator inputs for Duration/Infinite effects
 * 
 * 2. PreAttributeBaseChange():  
 *    - WHEN: Fires only for BaseValue changes (Instant/Periodic GameplayEffects)
 *    - PURPOSE: Clamps BaseValue to [0, MaxX] preventing "invisible buffer" overflows
 *    - KEY: Prevents BaseValue > MaxValue scenarios where hidden overflow exists
 * 
 * 3. PostGameplayEffectExecute():
 *    - WHEN: Fires after Instant/Periodic GameplayEffect execution completes
 *    - PURPOSE: Final authoritative clamping and reactive adjustments
 *    - SAFE: Can use SetX() methods here to modify attributes directly
 *    - USE CASE: Adjust current values when max values change, handle damage/healing resolution
 * 
 * CALLBACK ORDER CHEATSHEET:
 * - PreAttributeBaseChange → [GE applies to BaseValue] → PreAttributeChange → PostGameplayEffectExecute
 * - For Duration/Infinite GEs: Only PreAttributeChange → [GE modifies CurrentValue temporarily]
 * 
 * This multi-layered approach ensures no attribute can exceed its valid range through any pathway,
 * preventing the common "fire area + health crystals" bug where attributes appear clamped but have hidden overflow.
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
	 * Called before CurrentValue is modified for any attribute from any source.
	 * 
	 * WHEN IT FIRES:
	 * - Always: Before any CurrentValue change (Duration/Infinite GEs, direct sets, aggregator updates)
	 * - Does NOT fire for BaseValue-only changes (use PreAttributeBaseChange for those)
	 * 
	 * PURPOSE:
	 * - Clamp visible CurrentValue to valid ranges [0, Max] for consistency
	 * - React to Max attribute changes (e.g., when MaxHealth decreases, clamp Health accordingly)
	 * 
	 * IMPORTANT LIMITATION:
	 * - Clamping NewValue here does NOT persist to underlying aggregator inputs for Duration/Infinite effects
	 * - Example: If Health=90, MaxHealth=100, and a Duration GE tries to add +100 Health:
	 *   - NewValue gets clamped to 100 (visible), but the +100 modifier remains in the aggregator
	 *   - When MaxHealth increases to 150, the hidden +100 resurfaces, making Health jump to 190
	 * - This is why we need PostGameplayEffectExecute for final authoritative clamping
	 *
	 * @param Attribute The attribute about to change (CurrentValue)
	 * @param NewValue  The requested new CurrentValue (clamped in-place)
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * Called before BaseValue is modified for any attribute.
	 * 
	 * WHEN IT FIRES:
	 * - Only for BaseValue changes from Instant/Periodic GameplayEffects
	 * - Does NOT fire for Duration/Infinite effects (they modify CurrentValue via modifiers)
	 * - Does NOT fire for direct attribute sets (SetHealth, etc.)
	 * 
	 * PURPOSE:
	 * - Clamp BaseValue to [0, Max] preventing permanent overflow from Instant/Periodic effects
	 * - Prevent "invisible buffer" scenarios where BaseValue > MaxValue
	 * - Example: Health.BaseValue=150 with MaxHealth=100 creates a hidden 50-point buffer
	 * 
	 * CRITICAL FOR:
	 * - Instant healing potions that shouldn't create permanent overflow
	 * - Periodic effects (fire areas, health crystals) that apply permanent increments each tick
	 * - Any effect that permanently modifies the base attribute value
	 *
	 * @param Attribute The attribute about to change (BaseValue)
	 * @param NewValue  The requested new BaseValue (clamped in-place)
	 */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/**
	 * Called after a GameplayEffect has executed and applied its modifiers.
	 * 
	 * WHEN IT FIRES:
	 * - After Instant/Periodic GameplayEffects complete execution
	 * - Does NOT fire for Duration/Infinite effects (they don't "execute", they just apply/remove modifiers)
	 * - Server-authoritative: typically only runs on the server in networked games
	 * 
	 * PURPOSE:
	 * - Final authoritative clamping using SetX() methods (safe to modify attributes here)
	 * - React to attribute changes (adjust current values when max values change)
	 * - Handle damage/heal resolution, death checks, gameplay cues
	 * - Apply business logic that should happen after effects complete
	 * 
	 * REPLICATION NOTE:
	 * - This callback runs on the server; attribute changes replicate normally via RepNotify
	 * - Clients will receive the final clamped values through normal replication channels
	 * 
	 * TYPICAL USAGE:
	 * - If MaxHealth changes: SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()))
	 * - Handle meta-attributes (Damage): subtract from Health, then clamp Health
	 * - Broadcast UI updates, trigger gameplay cues, check for death conditions
	 *
	 * @param Data Contains information about the effect that just executed (attribute, new value, source, target, etc.)
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
