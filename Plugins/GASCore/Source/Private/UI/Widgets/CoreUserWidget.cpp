// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.
//
// Implementation notes:
// - SetWidgetController stores the reference and immediately calls the BP event hook.
// - Calling SetWidgetController repeatedly re-fires the event; ensure your BP logic is idempotent
//   or guards against double binding if needed.

#include "UI/Widgets/CoreUserWidget.h"

void UCoreUserWidget::SetWidgetController(UObject* InWidgetController)
{
	// Store the new Widget Controller reference (not owned; ensure lifetime is managed externally).
	WidgetController = InWidgetController;

	// Trigger the Blueprint event so the widget can bind to controller delegates and
	// perform any initial UI updates (e.g., broadcast initial values).
	OnWidgetControllerSet();
}