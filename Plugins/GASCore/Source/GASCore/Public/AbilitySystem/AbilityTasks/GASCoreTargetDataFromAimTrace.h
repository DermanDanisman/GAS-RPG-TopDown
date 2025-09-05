// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GASCoreTargetDataFromAimTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimTraceTargetDataSignature, const FHitResult&, HitResultData);

/**
 * 
 */
UCLASS()
class GASCORE_API UGASCoreTargetDataFromAimTrace : public UAbilityTask
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "GASCore|Ability Task|Target Data From Aim Trace", meta = (DisplayName = "TargetDataFromAimTrace", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UGASCoreTargetDataFromAimTrace* CreateTargetDataFromAimTrace(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FAimTraceTargetDataSignature ValidHitResultData;

private:

	virtual void Activate() override;
};
