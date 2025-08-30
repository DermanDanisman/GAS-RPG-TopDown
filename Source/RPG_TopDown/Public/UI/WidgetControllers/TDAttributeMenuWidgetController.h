// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h"
#include "TDAttributeMenuWidgetController.generated.h"

class UTDAttributeInfo;
struct FGASCoreAttributeInformation;

// These are BlueprintAssignable so widgets can bind in BP to receive updates.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FGASCoreAttributeInformation&, AttributeInfo);

/**
 * UTDAttributeMenuWidgetController
 *
 * Purpose:
 * - Drive the Attribute Menu UI with attribute data.
 * - Broadcast initial values once dependencies are valid.
 * - (Future) Bind to AbilitySystemComponent (ASC) change delegates for live updates.
 *
 * High-level flow:
 * - BroadcastInitialValues():
 *     1) Cast the base AttributeSet to your concrete UTDAttributeSet.
 *     2) For each (Tag → AttributeAccessor) entry in the AttributeSet registry:
 *         a) Look up UI metadata from UAttributeInfo (name, description, icon, etc.).
 *         b) Call the accessor to get FGameplayAttribute identity.
 *         c) Resolve the current numeric value from the AttributeSet.
 *         d) Fill FTDAttributeInfo and broadcast via AttributeInfoDelegate.
 * - BindCallbacksToDependencies():
 *     - Intended to subscribe to ASC attribute change delegates so the UI updates live.
 *     - Keeps the controller generic (no per-attribute boilerplate).
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
	 *
	 * Design note:
	 * - This implementation is generic: it loops over a Tag→Accessor registry
	 *   defined in the AttributeSet, so you don't need per-attribute code here.
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * Subscribe to attribute/value change delegates on the AbilitySystemComponent.
	 * Override from base to bind your attribute delegates and any custom ASC delegates.
	 *
	 * Recommended usage:
	 * - Iterate the same Tag→Accessor registry to subscribe to ASC change delegates
	 *   keyed by FGameplayAttribute. In the handler, rebuild FTDAttributeInfo
	 *   for the changed tag and rebroadcast.
	 */
	virtual void BindCallbacksToDependencies() override;

	/** Multicast event for UI rows; rows bind in BP and update when their Tag matches Info.Tag exactly. */
	UPROPERTY(BlueprintAssignable, Category="Top Down|Attribute Widget Controller|Delegates")
	FAttributeInfoSignature AttributeInfoDelegate;

protected:
	
	/**
	 * Data asset that maps FGameplayTag → display metadata (name, description, icon, formatting).
	 * The controller consults this for each broadcast so the UI renders consistent, data-driven labels.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Top Down|Attribute Widget Controller|Data Asset")
	TObjectPtr<UTDAttributeInfo> AttributeInfoDataAsset;

private:

	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag) const;
};