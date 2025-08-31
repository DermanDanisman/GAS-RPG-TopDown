// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Centralized registration and access for native Gameplay Tags.
 */
struct FTDGameplayTags
{
public:
	static const FTDGameplayTags& Get() { return TDGameplayTags; }
	static void InitializeNativeGameplayTags();

	// -----------------------------------------------------------------------------
	// Primary Attributes
	// -----------------------------------------------------------------------------
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Dexterity;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Endurance;
	FGameplayTag Attributes_Primary_Vigor;

	// -----------------------------------------------------------------------------
	// Secondary Attributes
	// -----------------------------------------------------------------------------
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_Evasion;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_MaxMana;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_StaminaRegeneration;
	FGameplayTag Attributes_Secondary_MaxStamina;

	// -----------------------------------------------------------------------------
	// Vital Attributes
	// -----------------------------------------------------------------------------
	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_MaxHealth;
	FGameplayTag Attributes_Vital_Mana;
	FGameplayTag Attributes_Vital_MaxMana;
	FGameplayTag Attributes_Vital_Stamina;
	FGameplayTag Attributes_Vital_MaxStamina;

	// -----------------------------------------------------------------------------
	// Inputs
	// -----------------------------------------------------------------------------
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_QuickSlot_1;
	FGameplayTag InputTag_QuickSlot_2;
	FGameplayTag InputTag_QuickSlot_3;
	FGameplayTag InputTag_QuickSlot_4;

private:
	static FTDGameplayTags TDGameplayTags;
};