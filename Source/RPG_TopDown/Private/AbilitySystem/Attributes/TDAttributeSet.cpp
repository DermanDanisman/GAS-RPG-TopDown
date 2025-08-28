#include "AbilitySystem/Attributes/TDAttributeSet.h"
#include "Net/UnrealNetwork.h"

UTDAttributeSet::UTDAttributeSet()
{
	// Register current↔max pairs once; base class handles all clamping
	RegisterCurrentMaxPair(GetHealthAttribute(), GetMaxHealthAttribute());
	RegisterCurrentMaxPair(GetManaAttribute(), GetMaxManaAttribute());
	RegisterCurrentMaxPair(GetStaminaAttribute(), GetMaxStaminaAttribute());
}

void UTDAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ===================
	// Primary Attributes 
	// ===================
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	// ===================
	// Secondary Attributes 
	// ===================

	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, StaminaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);

	// ===================
	// Vital Attributes
	// ===================
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
}

// =======================================
// Primary Attributes Rep Notify Functions
// =======================================

void UTDAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Strength, OldStrength);
}

void UTDAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Dexterity, OldDexterity);
}

void UTDAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Intelligence, OldIntelligence);
}

void UTDAttributeSet::OnRep_Endurance(const FGameplayAttributeData& OldEndurance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Endurance, OldEndurance);
}

void UTDAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Vigor, OldVigor);
}

// =======================================
// Secondary Attributes Rep Notify Functions
// =======================================

void UTDAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Armor, OldArmor);
}

void UTDAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UTDAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, BlockChance, OldBlockChance);
}

void UTDAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UTDAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UTDAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UTDAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UTDAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, MaxHealth, OldMaxHealth);
}

void UTDAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UTDAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, MaxMana, OldMaxMana);
}

void UTDAttributeSet::OnRep_StaminaRegeneration(const FGameplayAttributeData& OldStaminaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, StaminaRegeneration, OldStaminaRegeneration);
}

void UTDAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, MaxStamina, OldMaxStamina);
}

// =======================================
// Vital Attributes Rep Notify Functions
// =======================================

void UTDAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Health, OldHealth);
}

void UTDAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Mana, OldMana);
}

void UTDAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDAttributeSet, Stamina, OldStamina);
}
