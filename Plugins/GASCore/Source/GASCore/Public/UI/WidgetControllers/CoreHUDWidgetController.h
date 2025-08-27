// Copyright:
// © 2025 Heathrow (Derman). All rights reserved.
//
// Purpose:
// - Defines a Widget Controller that sits between the GAS data model (ASC + AttributeSet)
//   and your UUserWidget-based HUD. It follows an MVC-style pattern:
//   - Model: GAS (ASC, AttributeSet, GameplayEffects, GameplayTags)
//   - Controller: this class (reads from Model, broadcasts to View)
//   - View: Widgets (bind to controller delegates; display-only)
//
// Responsibilities:
// - Broadcast initial attribute values to the UI when the controller is set up
// - Subscribe to GAS attribute change delegates and rebroadcast values to widgets
// - Relay GameplayEffect asset tag events (via ASC) to the UI (e.g., messages/toasts)
// - Optionally look up UI message rows by gameplay tag in a DataTable
//
// Notes on lifetime and safety:
// - Bind callbacks only after AbilitySystemComponent and AttributeSet are valid and initialized
// - Lambdas capture `this`; ensure the controller outlives bindings or use weak captures
// - DataTable lookups assume the row key is the tag's FName (Tag.GetTagName())
//
// See also:
// - UI/WidgetControllers/CoreHUDWidgetController.cpp for implementation details
// - Components/CoreAbilitySystemComponent.* for the Effect Asset Tags delegate

#pragma once

#include "CoreMinimal.h"
#include "CoreWidgetController.h"
#include "CoreHUDWidgetController.generated.h"





/**
 * UCoreHUDWidgetController
 *
 * Bridges the GAS data model to HUD widgets:
 * - On setup, broadcasts initial attribute values so widgets can initialize their displays
 * - Subscribes to attribute change delegates, to push real-time updates
 * - Listens for GameplayEffect asset tags (from ASC) and forwards matching UI message rows
 */
UCLASS()
class GASCORE_API UCoreHUDWidgetController : public UCoreWidgetController
{
	GENERATED_BODY()

public:
};