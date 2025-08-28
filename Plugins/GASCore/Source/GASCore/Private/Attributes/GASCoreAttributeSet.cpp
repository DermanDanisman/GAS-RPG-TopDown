// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// File: GASCoreAttributeSet.cpp (implementation)
// Purpose:
//   - Implement clamping and rounding logic declared in UGASCoreAttributeSet.
//   - Provide effect-context population and attribute data helpers.
//
// Implementation notes:
//   - Rounding occurs after clamping to ensure final persisted values respect both constraints.
//   - PostGameplayEffectExecute re-clamps Current when its paired Max changed and writes the rounded value.
//   - Helpers access FGameplayAttributeData via property reflection to read/write numeric values.

#include "Attributes/GASCoreAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"

UGASCoreAttributeSet::UGASCoreAttributeSet()
{
}

void UGASCoreAttributeSet::RegisterCurrentMaxPair(const FGameplayAttribute& Current, const FGameplayAttribute& Max)
{
	// Register both directions for fast lookup.
	if (Current.IsValid() && Max.IsValid())
	{
		CurrentToMax.Add(Current, Max);
		MaxToCurrent.Add(Max, Current);
	}
}

bool UGASCoreAttributeSet::TryGetMaxForCurrent(const FGameplayAttribute& Current, FGameplayAttribute& OutMax) const
{
	if (const FGameplayAttribute* Found = CurrentToMax.Find(Current))
	{
		OutMax = *Found;
		return true;
	}
	return false;
}

bool UGASCoreAttributeSet::TryGetCurrentForMax(const FGameplayAttribute& Max, FGameplayAttribute& OutCurrent) const
{
	if (const FGameplayAttribute* Found = MaxToCurrent.Find(Max))
	{
		OutCurrent = *Found;
		return true;
	}
	return false;
}

const FGameplayAttributeData* UGASCoreAttributeSet::FindAttributeDataConst(const FGameplayAttribute& Attr, const UAttributeSet* Set)
{
	// All attributes on UAttributeSet are FGameplayAttributeData fields; use reflection to access storage.
	if (!Attr.IsValid()) return nullptr;
	if (FProperty* Prop = Attr.GetUProperty())
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			if (StructProp->Struct == TBaseStructure<FGameplayAttributeData>::Get())
			{
				return StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(Set);
			}
		}
	}
	return nullptr;
}

FGameplayAttributeData* UGASCoreAttributeSet::FindAttributeData(const FGameplayAttribute& Attr, UAttributeSet* Set)
{
	// Mutable access to the same attribute storage via reflection.
	if (!Attr.IsValid()) return nullptr;
	if (FProperty* Prop = Attr.GetUProperty())
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			if (StructProp->Struct == TBaseStructure<FGameplayAttributeData>::Get())
			{
				return StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(Set);
			}
		}
	}
	return nullptr;
}

float UGASCoreAttributeSet::GetCurrentNumeric(const FGameplayAttribute& Attr) const
{
	// Safe read of CurrentValue for the given attribute belonging to this set.
	if (const FGameplayAttributeData* Data = FindAttributeDataConst(Attr, this))
	{
		return Data->GetCurrentValue();
	}
	return 0.f;
}

float UGASCoreAttributeSet::GetBaseNumeric(const FGameplayAttribute& Attr) const
{
	// Safe read of BaseValue for the given attribute belonging to this set.
	if (const FGameplayAttributeData* Data = FindAttributeDataConst(Attr, this))
	{
		return Data->GetBaseValue();
	}
	return 0.f;
}

float UGASCoreAttributeSet::RoundToDecimals(float Value, int32 Decimals)
{
	// Half-away-from-zero rounding.
	if (Decimals <= 0)
	{
		return static_cast<float>(FMath::RoundToInt(Value));
	}
	const float Scale = FMath::Pow(10.f, static_cast<float>(Decimals));
	return static_cast<float>(FMath::RoundToInt(Value * Scale)) / Scale;
}

void UGASCoreAttributeSet::SetCurrentNumeric(const FGameplayAttribute& Attr, float NewValue)
{
	// Apply rounding policy before persisting the value via the owning ASC.
	const int32 Decimals = GetRoundingDecimals(Attr);
	const float Rounded = RoundToDecimals(NewValue, Decimals);

	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		// Use SetNumericAttributeBase to update the authoritative base value for the attribute.
		ASC->SetNumericAttributeBase(Attr, Rounded);
	}
}

void UGASCoreAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// If this attribute is registered as a Current with a paired Max, clamp to [0, Max].
	FGameplayAttribute MaxAttr;
	if (TryGetMaxForCurrent(Attribute, MaxAttr))
	{
		const float Max = GetCurrentNumeric(MaxAttr);
		const float Old = GetCurrentNumeric(Attribute);

		// Clamp first, then round so the final value honors both constraints.
		NewValue = FMath::Clamp(NewValue, 0.f, Max);
		const int32 Decimals = GetRoundingDecimals(Attribute);
		NewValue = RoundToDecimals(NewValue, Decimals);

		if (!FMath::IsNearlyEqual(Old, NewValue))
		{
			OnCurrentClampedByMax(Attribute, MaxAttr, Old, NewValue);
		}
	}
	else
	{
		// Even if not paired, you may want rounding for consistency (e.g., Strength as an integer).
		const int32 Decimals = GetRoundingDecimals(Attribute);
		NewValue = RoundToDecimals(NewValue, Decimals);
	}
}

void UGASCoreAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	// For BaseValue changes, clamp to [0, Max(Current)] if a pair exists.
	FGameplayAttribute MaxAttr;
	if (TryGetMaxForCurrent(Attribute, MaxAttr))
	{
		const float Max = GetCurrentNumeric(MaxAttr);
		NewValue = FMath::Clamp(NewValue, 0.f, Max);
	}

	// Apply rounding to Base as well, so replicated Base matches your policy.
	const int32 Decimals = GetRoundingDecimals(Attribute);
	NewValue = RoundToDecimals(NewValue, Decimals);
}

void UGASCoreAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// If the attribute that just changed is a Max, ensure the paired Current is clamped and rounded.
	FGameplayAttribute CurrentAttr;
	if (TryGetCurrentForMax(Data.EvaluatedData.Attribute, CurrentAttr))
	{
		const float Max = GetCurrentNumeric(Data.EvaluatedData.Attribute);
		const float OldCurrent = GetCurrentNumeric(CurrentAttr);

		// Clamp then round the Current value.
		float NewCurrent = FMath::Clamp(OldCurrent, 0.f, Max);
		const int32 Decimals = GetRoundingDecimals(CurrentAttr);
		NewCurrent = RoundToDecimals(NewCurrent, Decimals);

		// Only write if it actually changed to avoid unnecessary churn.
		if (!FMath::IsNearlyEqual(OldCurrent, NewCurrent))
		{
			SetCurrentNumeric(CurrentAttr, NewCurrent);
			OnMaxAttributeChangedAndClamped(CurrentAttr, Data.EvaluatedData.Attribute, OldCurrent, NewCurrent);
		}
	}

	// If you need source/target info (for cues or logging), populate the effect context:
	// FGASCoreEffectContext Ctx; PopulateCoreEffectContext(Data, Ctx);
}

void UGASCoreAttributeSet::PopulateCoreEffectContext(const FGameplayEffectModCallbackData& Data,
	FGASCoreEffectContext& InEffectContext) const
{
	// Source = causer of the effect, Target = owner of this ASC.
	InEffectContext.GameplayEffectContextHandle = Data.EffectSpec.GetContext();

	// ----- Source side -----
	InEffectContext.SourceASC = InEffectContext.GameplayEffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	if (IsValid(InEffectContext.SourceASC) && InEffectContext.SourceASC->AbilityActorInfo.IsValid())
	{
		InEffectContext.SourceAvatarActor = InEffectContext.SourceASC->GetAvatarActor();
		InEffectContext.SourceController   = InEffectContext.SourceASC->AbilityActorInfo->PlayerController.Get();

		// Fallback to pawn->controller if PlayerController is null (e.g., AI).
		if (!InEffectContext.SourceController && InEffectContext.SourceAvatarActor)
		{
			if (const APawn* Pawn = Cast<APawn>(InEffectContext.SourceAvatarActor))
			{
				InEffectContext.SourceController = Pawn->GetController();
			}
		}

		// Prefer controller->GetCharacter; else cast avatar directly.
		if (InEffectContext.SourceController)
		{
			InEffectContext.SourceCharacter = InEffectContext.SourceController->GetCharacter();
		}
		else if (InEffectContext.SourceAvatarActor)
		{
			InEffectContext.SourceCharacter = Cast<ACharacter>(InEffectContext.SourceAvatarActor);
		}
	}

	// ----- Target side -----
	if (Data.Target.AbilityActorInfo.IsValid() && IsValid(Data.Target.GetAvatarActor()))
	{
		InEffectContext.TargetAvatarActor = Data.Target.GetAvatarActor();
		InEffectContext.TargetController  = Data.Target.AbilityActorInfo->PlayerController.Get();

		// Prefer controller->GetCharacter; else derive from pawn.
		if (InEffectContext.TargetController)
		{
			InEffectContext.TargetCharacter = InEffectContext.TargetController->GetCharacter();
		}
		else
		{
			if (APawn* TargetPawn = Cast<APawn>(InEffectContext.TargetAvatarActor))
			{
				InEffectContext.TargetController = TargetPawn->GetController();
				InEffectContext.TargetCharacter  = Cast<ACharacter>(TargetPawn);
			}
		}

		// Prefer pulling ASC directly from AbilityActorInfo; fallback via BP library.
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