// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreWidgetController.h"
#include "CoreHUDWidgetController.generated.h"

// Declare multicast delegates for different HUD attribute changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, NewMaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedSignature, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);

/**
 * 
 */
UCLASS()
class GASCORE_API UCoreHUDWidgetController : public UCoreWidgetController
{
	GENERATED_BODY()

public:

	/** Called to broadcast the initial values of attributes to the UI. Should be overridden in child classes. */
	virtual void BroadcastInitialValues() override;

	/** Called to bind attribute change callbacks/delegates to the ability system. Should be overridden in child classes. */
	virtual void BindCallbacksToDependencies() override;
	
	/** Delegate for listening to health value changes. */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	/** Delegate for listening to max health changes. */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	/** Delegate for listening to mana value changes. */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnManaChangedSignature OnManaChanged;

	/** Delegate for listening to max mana changes. */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnMaxManaChangedSignature OnMaxManaChanged;

	/** Delegate for listening to stamina value changes. */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnStaminaChangedSignature OnStaminaChanged;

	/** Delegate for listening to stamina value changes. */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnMaxStaminaChangedSignature OnMaxStaminaChanged;

protected:

	UPROPERTY(EditDefaultsOnly, Category="HUD Widget Controller|UI")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
};

template <typename T>
T* UCoreHUDWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	if (IsValid(DataTable))
	{
		T* MessageRow = DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
		if (IsValid(MessageRow))
			return MessageRow;
	}
	return nullptr;
}
