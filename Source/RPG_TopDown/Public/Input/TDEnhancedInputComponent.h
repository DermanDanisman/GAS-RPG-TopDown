// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Input/GASCoreEnhancedInputComponent.h"
#include "TDEnhancedInputComponent.generated.h"

/**
 * Custom input component that binds ability input actions from a data-driven config.
 * Provides template-based binding for input delegates using tags.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_TOPDOWN_API UTDEnhancedInputComponent : public UGASCoreEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UTDEnhancedInputComponent();
};