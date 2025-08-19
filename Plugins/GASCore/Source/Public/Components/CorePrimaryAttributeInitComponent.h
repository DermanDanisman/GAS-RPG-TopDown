// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CorePrimaryAttributeInitComponent.generated.h"


class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASCORE_API UCorePrimaryAttributeInitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCorePrimaryAttributeInitComponent();

	// Gameplay Effect that sets initial primary attributes (e.g., STR/DEX/INT/END/VIG).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GASCore|Init")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	// Explicitly initialize attributes. If TargetASC is null, resolves from Owner.
	UFUNCTION(BlueprintCallable, Category="GASCore|Init")
	void InitializePrimaryAttributes(UAbilitySystemComponent* TargetAbilitySystemComponent = nullptr) const;


	
};
