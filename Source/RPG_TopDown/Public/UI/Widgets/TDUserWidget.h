// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GASCoreUI/Public/UI/Widgets/GASCoreUIUserWidget.h"
#include "TDUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class RPG_TOPDOWN_API UTDUserWidget : public UGASCoreUIUserWidget
{
	GENERATED_BODY()

public:

	virtual void SetWidgetController(UObject* InWidgetController) override;
};
