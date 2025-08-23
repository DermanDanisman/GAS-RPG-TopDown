// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Components/CoreDefaultAttributeInitComponent.h"
#include "TDDefaultAttributeInitComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_TOPDOWN_API UTDDefaultAttributeInitComponent : public UCoreDefaultAttributeInitComponent
{
	GENERATED_BODY()

public:
	
	UTDDefaultAttributeInitComponent();

	virtual void InitializeDefaultAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent) const override;
	
};
