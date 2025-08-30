// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Module Includes =====
#include "CoreMinimal.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h"

// Forward declarations to keep compile-time dependencies minimal
class UTDAttributeInfo;
struct FGASCoreAttributeInformation;
struct FGameplayTag; // used by the private helper signature below

#include "TDAttributeMenuWidgetController.generated.h"

// ===== Delegates =====
// These are BlueprintAssignable so widgets can bind in BP to receive updates.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FGASCoreAttributeInformation&, AttributeInfo);

/**
 * UTDAttributeMenuWidgetController
 *
 * Purpose:
 * - Drive the Attribute Menu UI with attribute data sourced from a Data Asset.
 * - Broadcast initial values once dependencies are valid.
 * - Bind to AbilitySystemComponent (ASC) change delegates for live updates.
 *
 * High-level flow:
 * - BroadcastInitialValues():
 *     1) Iterate the UI rows defined in the Data Asset.
 *     2) Validate the row's AttributeGetter (FGameplayAttribute identity).
 *     3) Compute the current numeric value via AttributeGetter.GetNumericValue(AttributeSet).
 *     4) Broadcast a filled FGASCoreAttributeInformation to the UI.
 *
 * - BindCallbacksToDependencies():
 *     - For each row, subscribe to ASC's value change delegate keyed by the row's FGameplayAttribute.
 *     - On change, re-broadcast only that row (minimal UI update).
 *
 * Notes:
 * - This controller remains generic; adding/removing attributes is handled by editing the Data Asset.
 * - Widgets filter or react to updates by comparing their Tag to the incoming AttributeInfo.AttributeTag.
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_TOPDOWN_API UTDAttributeMenuWidgetController : public UGASCoreUIWidgetController
{
	GENERATED_BODY()

public:
	// ===== UGASCoreUIWidgetController overrides =====

	/**
	 * Broadcast initial values to the UI once the controller has valid references
	 * (PlayerController, PlayerState, ASC, AttributeSet, and AttributeInfoDataAsset).
	 * Generic implementation that loops the Data Asset rows.
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * Subscribe to ASC attribute/value change delegates for live updates.
	 * Binds one delegate per configured FGASCoreAttributeInformation row (via AttributeGetter).
	 */
	virtual void BindCallbacksToDependencies() override;

	// ===== Delegates (UI consumption) =====

	/** Multicast event for UI rows; widgets bind in BP and update when their Tag matches Info.AttributeTag. */
	UPROPERTY(BlueprintAssignable, Category="Top Down|Attribute Widget Controller|Delegates")
	FAttributeInfoSignature AttributeInfoDelegate;

protected:
	// ===== Data sources =====
	
	/**
	 * Data Asset that maps FGameplayTag → display metadata (name, description, icon/formatting)
	 * and carries the FGameplayAttribute identity (AttributeGetter) per row.
	 * The controller consults this asset for both initial and change broadcasts.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Top Down|Attribute Widget Controller|Data Asset")
	TObjectPtr<UTDAttributeInfo> AttributeInfoDataAsset = nullptr;

private:
	// ===== Internal helpers =====

	/**
	 * Look up metadata by AttributeTag, compute the current numeric value via the row's AttributeGetter,
	 * and broadcast a filled FGASCoreAttributeInformation to UI listeners.
	 *
	 * Expected to be called:
	 * - During initial broadcast (for each row).
	 * - From ASC change delegates (only for the changed row).
	 */
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag) const;
};