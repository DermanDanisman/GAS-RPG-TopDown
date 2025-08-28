// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h"
#include "TDAttributeMenuWidgetController.generated.h"

/**
 * UTDAttributeMenuWidgetController
 *
 * Minimal controller class for Attribute Menu widgets.
 * Extends UGASCoreUIWidgetController and provides empty overrides
 * for the required abstract methods. Projects can later replace/extend
 * this implementation with specific attribute menu functionality.
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
};