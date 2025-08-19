// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Attributes/CoreAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UCoreAttributeSet::UCoreAttributeSet()
{
	InitHealth(25.f);
	InitMaxHealth(100.f);
	InitMana(10.f);
	InitMaxMana(50.f);
	InitStamina(25.f);
	InitMaxStamina(80.f);
}

void UCoreAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ===================
	// Primary Attributes 
	// ===================
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	// ===================
	// Vital Attributes
	// ===================
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void UCoreAttributeSet::PopulateEffectContext(const struct FGameplayEffectModCallbackData& Data,
	FCoreEffectContext& InEffectContext) const
{
	// Source = causer of the effect, Target = owner of this ASC
	InEffectContext.GameplayEffectContextHandle = Data.EffectSpec.GetContext();

	// Source side
	InEffectContext.SourceASC = InEffectContext.GameplayEffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	if (IsValid(InEffectContext.SourceASC) && InEffectContext.SourceASC->AbilityActorInfo.IsValid())
	{
		InEffectContext.SourceAvatarActor = InEffectContext.SourceASC->GetAvatarActor();
		InEffectContext.SourceController = InEffectContext.SourceASC->AbilityActorInfo->PlayerController.Get();

		// Fallback to pawn->controller if PlayerController is null (e.g., AI)
		if (!InEffectContext.SourceController && InEffectContext.SourceAvatarActor)
		{
			if (const APawn* Pawn = Cast<APawn>(InEffectContext.SourceAvatarActor))
			{
				InEffectContext.SourceController = Pawn->GetController();
			}
		}

		// Derive character if controller exists, else try casting avatar
		if (InEffectContext.SourceController)
		{
			InEffectContext.SourceCharacter = InEffectContext.SourceController->GetCharacter();
		}
		else if (InEffectContext.SourceAvatarActor)
		{
			InEffectContext.SourceCharacter = Cast<ACharacter>(InEffectContext.SourceAvatarActor);
		}
	}

	// Target side
	if (Data.Target.AbilityActorInfo.IsValid() && IsValid(Data.Target.GetAvatarActor()))
	{
		InEffectContext.TargetAvatarActor = Data.Target.GetAvatarActor();
		InEffectContext.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();

		// Safely get character and fallback via pawn/controller as needed
		if (InEffectContext.TargetController)
		{
			InEffectContext.TargetCharacter = InEffectContext.TargetController->GetCharacter();
		}
		else
		{
			if (APawn* TargetPawn = Cast<APawn>(InEffectContext.TargetAvatarActor))
			{
				InEffectContext.TargetController = TargetPawn->GetController();
				InEffectContext.TargetCharacter = Cast<ACharacter>(TargetPawn);
			}
		}

		// Prefer pulling ASC directly from AbilityActorInfo when available
		if (Data.Target.AbilityActorInfo->AbilitySystemComponent.IsValid())
		{
			InEffectContext.TargetASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
		}
		else
		{
			InEffectContext.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InEffectContext.TargetAvatarActor);
		}
	}
}

void UCoreAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp Health between 0 and MaxHealth
	// This handles CurrentValue changes from Duration/Infinite GEs, direct sets, and aggregator updates
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
	}
	// Clamp Mana between 0 and MaxMana
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxMana());
	}
	// Clamp Stamina between 0 and MaxStamina
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxStamina());
	}
}

void UCoreAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// Clamp Health.BaseValue between 0 and MaxHealth
	// This prevents "invisible buffer" overflow from Instant/Periodic GameplayEffects
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
	}
	// Clamp Mana.BaseValue between 0 and MaxMana
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxMana());
	}
	// Clamp Stamina.BaseValue between 0 and MaxStamina
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxStamina());
	}
	
	// Note: When Max values change, you typically also adjust current values here or in PostGameplayEffectExecute
	// (e.g., if MaxHealth decreases below Health, clamp Health down to new Max).
}

void UCoreAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FCoreEffectContext EffectProperties;
	PopulateEffectContext(Data, EffectProperties);
	
	// FINAL AUTHORITATIVE CLAMPING:
	// This is the safest place to use SetX() methods to modify attributes directly.
	// Handle cases where Max attributes changed, requiring current values to be adjusted.
	
	// When MaxHealth changes, ensure Health doesn't exceed the new maximum
	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	// When MaxMana changes, ensure Mana doesn't exceed the new maximum
	else if (Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	// When MaxStamina changes, ensure Stamina doesn't exceed the new maximum
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}

	// Optional: Broadcast cues or UI updates using EffectProperties here if needed.
}

// =======================================
// Primary Attributes Rep Notify Functions
// =======================================

void UCoreAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Strength, OldStrength);
}

void UCoreAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Dexterity, OldDexterity);
}

void UCoreAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Intelligence, OldIntelligence);
}

void UCoreAttributeSet::OnRep_Endurance(const FGameplayAttributeData& OldEndurance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Endurance, OldEndurance);
}

void UCoreAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Vigor, OldVigor);
}

// =======================================
// Vital Attributes Rep Notify Functions
// =======================================

/** RepNotify: Health value updated on clients */
void UCoreAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Health, OldHealth);
}

/** RepNotify: MaxHealth value updated on clients */
void UCoreAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, MaxHealth, OldMaxHealth);
}

/** RepNotify: Mana value updated on clients */
void UCoreAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Mana, OldMana);
}

/** RepNotify: MaxMana value updated on clients */
void UCoreAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, MaxMana, OldMaxMana);
}

/** RepNotify: Stamina value updated on clients */
void UCoreAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, Stamina, OldStamina);
}

/** RepNotify: MaxStamina value updated on clients */
void UCoreAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreAttributeSet, MaxStamina, OldMaxStamina);
}