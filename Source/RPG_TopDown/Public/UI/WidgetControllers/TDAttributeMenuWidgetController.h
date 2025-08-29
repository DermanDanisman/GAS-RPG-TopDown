// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "TDAttributeMenuWidgetController.generated.h"

// These are BlueprintAssignable so widgets can bind in BP to receive updates.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/** Delegate signature for attribute info changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeInfoChangedSignature, const FTDAttributeInfo&, AttributeInfo);

/**
 * UTDAttributeMenuWidgetController
 * 
 * Manages attribute display for menus/character sheets.
 * Handles initial value broadcasting and live updates via ASC delegates.
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_TOPDOWN_API UTDAttributeMenuWidgetController : public UGASCoreUIWidgetController
{
	GENERATED_BODY()

public:

	/**
	 * Broadcast initial values to the UI.
	 * Called once the controller has valid references (PlayerController, PlayerState, ASC, AttributeSet).
	 * Override from base to push initial attribute values to widgets.
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * Subscribe to attribute/value change delegates on the AbilitySystemComponent.
	 * Override from base to bind your attribute delegates and any custom ASC delegates.
	 */
	virtual void BindCallbacksToDependencies() override;

	/** Delegate for listening to Health value changes (NewHealth). */
	UPROPERTY(BlueprintAssignable, Category="GASCore|HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	/** Generic delegate for broadcasting attribute info changes */
	UPROPERTY(BlueprintAssignable, Category = "GASCore|Attribute Menu|Delegates")
	FOnAttributeInfoChangedSignature OnAttributeInfoChanged;

protected:
	/** Data Asset containing attribute metadata for lookup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute Info")
	TObjectPtr<UAttributeInfo> AttributeInfoDataAsset;

private:
	/**
	 * Helper method to broadcast attribute information for a specific tag and attribute.
	 * Looks up metadata from the data asset and broadcasts the complete info.
	 * @param AttributeTag - The gameplay tag identifying the attribute
	 * @param Attribute - The FGameplayAttribute for reading current value
	 */
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
};
