// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "TDGameplayTags.h"
#include "GameplayTagsManager.h"

FTDGameplayTags FTDGameplayTags::TDGameplayTags;

void FTDGameplayTags::InitializeNativeGameplayTags()
{
	// -----------------------------------------------------------------------------
	// Primary Attributes
	// -----------------------------------------------------------------------------
	TDGameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Primary.Strength")),
		TEXT("Increases physical damage"));
	
	TDGameplayTags.Attributes_Primary_Dexterity = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Primary.Dexterity")),
		TEXT("Increases attack speed, movement speed and evasion chance"));
	
	TDGameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Primary.Intelligence")),
		TEXT("Increases mana and magical damage"));
	
	TDGameplayTags.Attributes_Primary_Endurance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Primary.Endurance")),
		TEXT("Increases load capacity and stamina"));
	
	TDGameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Primary.Vigor")),
		TEXT("Increases resilience and health"));

	// -----------------------------------------------------------------------------
	// Secondary Attributes
	// -----------------------------------------------------------------------------
	TDGameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.Armor")),
		TEXT("Mitigates incoming physical damage; often scales from Endurance/Resilience"));
	
	TDGameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.ArmorPenetration")),
		TEXT("Reduces target's effective armor; improves damage vs armored targets"));
	
	TDGameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.BlockChance")),
		TEXT("Chance to block incoming attacks; can scale from Armor"));

	TDGameplayTags.Attributes_Secondary_Evasion = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.Evasion")),
	TEXT("Chance to evade incoming attacks; can scale from Dexterity"));
	
	TDGameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.CriticalHitChance")),
		TEXT("Chance for attacks to crit; typically scales from Armor Penetration/Dexterity"));
	
	TDGameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.CriticalHitDamage")),
		TEXT("Crit damage multiplier or bonus; often scales from Armor Penetration"));
	
	TDGameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.CriticalHitResistance")),
		TEXT("Reduces chance or impact of incoming crits; scales from Armor"));
	
	TDGameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.MaxHealth")),
		TEXT("Maximum health pool; typically derived from Vigor"));
	
	TDGameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.HealthRegeneration")),
		TEXT("Health per second; typically scales from Vigor"));
	
	TDGameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.MaxMana")),
		TEXT("Maximum mana pool; typically derived from Intelligence"));
	
	TDGameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.ManaRegeneration")),
		TEXT("Mana per second; typically scales from Intelligence"));
	
	TDGameplayTags.Attributes_Secondary_StaminaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.StaminaRegeneration")),
		TEXT("Stamina per second; typically scales from Endurance"));
	
	TDGameplayTags.Attributes_Secondary_MaxStamina = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Secondary.MaxStamina")),
		TEXT("Maximum stamina pool; typically derived from Endurance"));

	// -----------------------------------------------------------------------------
	// Vital Attributes
	// -----------------------------------------------------------------------------
	TDGameplayTags.Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Vital.Health")),
		TEXT(""));
	
	TDGameplayTags.Attributes_Vital_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Vital.Mana")),
		TEXT(""));
	
	TDGameplayTags.Attributes_Vital_Stamina = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Attributes.Vital.Stamina")),
		TEXT(""));
}