// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/CoreUserWidget.h"

void UCoreUserWidget::SetWidgetController(UObject* InWidgetController)
{
	// Store the new Widget Controller reference.
	WidgetController = InWidgetController;
	// Trigger the Blueprint event for UI initialization or data binding.
	OnWidgetControllerSet();
}