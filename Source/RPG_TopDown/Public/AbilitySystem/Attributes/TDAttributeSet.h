#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Attributes/GASCoreAttributeSet.h"
#include "TDAttributeSet.generated.h"

/**
 * UTDAttributeSet
 *
 * Game-specific AttributeSet that:
 * - Declares primary, secondary, and vital attributes.
 * - Uses RepNotify to propagate server-authoritative changes to clients.
 * - In the constructor (see .cpp), registers Current↔Max pairs and fills the
 *   Tag→Accessor registry which powers generic UI broadcasting.
 *
 * Replication note:
 * - We use REPNOTIFY_Always so even "equivalent" updates still trigger RepNotifies
 *   (important for client prediction reconciliation and UI consistency).
 */
UCLASS()
class RPG_TOPDOWN_API UTDAttributeSet : public UGASCoreAttributeSet
{
	GENERATED_BODY()

public:
	UTDAttributeSet();

	/**
	 * Registers attributes for replication with RepNotify. Using REPNOTIFY_Always ensures
	 * OnRep handlers fire even when values appear unchanged locally (important for prediction/UI).
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// =========================
	// Primary Attributes
	// =========================

	/** Current Strength value - how much Strength the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Strength, Category="GASCore|Attributes|Primary")
	FGameplayAttributeData Strength;
	/** Generates attribute accessors (getter/setter/initter) for Strength */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Strength);

	/** Current Dexterity value - how much Dexterity the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Dexterity, Category="GASCore|Attributes|Primary")
	FGameplayAttributeData Dexterity;
	/** Generates attribute accessors (getter/setter/initter) for Dexterity */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Dexterity);

	/** Current Intelligence value - how much Intelligence the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Intelligence, Category="GASCore|Attributes|Primary")
	FGameplayAttributeData Intelligence;
	/** Generates attribute accessors (getter/setter/initter) for Intelligence */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Intelligence);

	/** Current Endurance value - how much Endurance the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Endurance, Category="GASCore|Attributes|Primary")
	FGameplayAttributeData Endurance;
	/** Generates attribute accessors (getter/setter/initter) for Endurance */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Endurance);

	/** Current Vigor value - how much Vigor the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Vigor, Category="GASCore|Attributes|Primary")
	FGameplayAttributeData Vigor;
	/** Generates attribute accessors (getter/setter/initter) for Vigor */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Vigor);

	
	// =======================================
	// Primary Attributes Rep Notify Functions
	// =======================================

	/**
	 * RepNotify for Strength attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	/**
	 * RepNotify for Dexterity attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Dexterity(const FGameplayAttributeData& OldDexterity) const;

	/**
	 * RepNotify for Intelligence attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	
	/**
	 * RepNotify for Endurance attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Endurance(const FGameplayAttributeData& OldEndurance) const;

	/**
	 * RepNotify for Vigor attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

	// =========================
	// Secondary Attributes
	// =========================

	/** Current Armor value - how much Armor the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Armor, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData Armor;
	/** Generates attribute accessors (getter/setter/initter) for Armor */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Armor);

	/** Current ArmorPenetration value - how much ArmorPenetration the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_ArmorPenetration, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData ArmorPenetration;
	/** Generates attribute accessors (getter/setter/initter) for ArmorPenetration */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, ArmorPenetration);

	/** Current BlockChance value - how much BlockChance the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_BlockChance, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData BlockChance;
	/** Generates attribute accessors (getter/setter/initter) for BlockChance */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, BlockChance);

	/** Current CriticalHitChance value - how much CriticalHitChance the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalHitChance, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData CriticalHitChance;
	/** Generates attribute accessors (getter/setter/initter) for CriticalHitChance */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, CriticalHitChance);

	/** Current CriticalHitDamage value - how much CriticalHitDamage the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalHitDamage, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData CriticalHitDamage;
	/** Generates attribute accessors (getter/setter/initter) for CriticalHitDamage */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, CriticalHitDamage);

	/** Current CriticalHitResistance value - how much CriticalHitResistance the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalHitResistance, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData CriticalHitResistance;
	/** Generates attribute accessors (getter/setter/initter) for CriticalHitResistance */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, CriticalHitResistance);
	
	/** Maximum health value - upper bound for Health */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="GASCore|Attributes|Vitals")
	FGameplayAttributeData MaxHealth;
	/** Generates attribute accessors for MaxHealth */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, MaxHealth);

	/** Current HealthRegeneration value - how much HealthRegeneration the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HealthRegeneration, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData HealthRegeneration;
	/** Generates attribute accessors (getter/setter/initter) for HealthRegeneration */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, HealthRegeneration);

	/** Maximum mana value - upper bound for Mana */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="GASCore|Attributes|Vitals")
	FGameplayAttributeData MaxMana;
	/** Generates attribute accessors for MaxMana */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, MaxMana);

	/** Current ManaRegeneration value - how much ManaRegeneration the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_ManaRegeneration, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData ManaRegeneration;
	/** Generates attribute accessors (getter/setter/initter) for ManaRegeneration */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, ManaRegeneration);

	/** Maximum stamina value - upper bound for Stamina */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxStamina, Category="GASCore|Attributes|Vitals")
	FGameplayAttributeData MaxStamina;
	/** Generates attribute accessors for MaxStamina */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, MaxStamina);

	/** Current StaminaRegeneration value - how much StaminaRegeneration the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_StaminaRegeneration, Category="GASCore|Attributes|Secondary")
	FGameplayAttributeData StaminaRegeneration;
	/** Generates attribute accessors (getter/setter/initter) for StaminaRegeneration */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, StaminaRegeneration);

	// =======================================
	// Secondary Attributes Rep Notify Functions
	// =======================================

	/** RepNotify for Armor attribute */
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	/** RepNotify for ArmorPenetration attribute */
	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;

	/** RepNotify for BlockChance attribute */
	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	/** RepNotify for CriticalHitChance attribute */
	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;

	/** RepNotify for CriticalHitDamage attribute */
	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;
	
	/** RepNotify for CriticalHitResistance attribute */
	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;

	/** RepNotify for HealthRegeneration attribute */
	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;
	
	/** RepNotify for MaxHealth attribute */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	/** RepNotify for ManaRegeneration attribute */
	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

	/** RepNotify for MaxMana attribute */
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	/** RepNotify for StaminaRegeneration attribute */
	UFUNCTION()
	void OnRep_StaminaRegeneration(const FGameplayAttributeData& OldStaminaRegeneration) const;

	/** RepNotify for MaxStamina attribute */
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;
	
	// =========================
	// Vital Attributes
	// =========================

	/** Current health value - how much health the character currently has */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="GASCore|Attributes|Vitals")
	FGameplayAttributeData Health;
	/** Generates attribute accessors (getter/setter/initter) for Health */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Health);

	/** Current mana value - resource for casting abilities */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="GASCore|Attributes|Vitals")
	FGameplayAttributeData Mana;
	/** Generates attribute accessors for Mana */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Mana);

	/** Current stamina value - resource for physical actions like sprinting */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Stamina, Category="GASCore|Attributes|Vitals")
	FGameplayAttributeData Stamina;
	/** Generates attribute accessors for Stamina */
	ATTRIBUTE_ACCESSORS(UTDAttributeSet, Stamina);

	// =======================================
	// Vital Attributes Rep Notify Functions
	// =======================================
	
	/**
	 * RepNotify for Health attribute. Triggers attribute change delegates and
	 * ensures clients update UI/prediction based on server authoritative values.
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	/** RepNotify for Mana attribute */
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
	
	/** RepNotify for Stamina attribute */
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;
};