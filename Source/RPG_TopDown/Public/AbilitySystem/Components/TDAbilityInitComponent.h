// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Components/GASCoreAbilityInitComponent.h"
#include "TDAbilityInitComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_TOPDOWN_API UTDAbilityInitComponent : public UGASCoreAbilityInitComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTDAbilityInitComponent();
};
