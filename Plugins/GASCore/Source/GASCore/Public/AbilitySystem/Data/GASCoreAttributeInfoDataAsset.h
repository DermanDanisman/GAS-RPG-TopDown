// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GASCoreAttributeInfoDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FGASCoreAttributeInformation
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();

	UPROPERTY(BlueprintReadOnly) // set at runtime before broadcast
	float AttributeValue = 0.f;

	// Select the attribute identity directly in the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute AttributeGetter;
};


/**
 * 
 */
UCLASS()
class GASCORE_API UGASCoreAttributeInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	TArray<FGASCoreAttributeInformation> GetAttributeInformation() const { return AttributeInformation; }

	// Optional: BP helper to fetch the array without exposing the property directly
	UFUNCTION(BlueprintPure, Category="GASCore|Attribute Info")
	void GetAttributeInformation_BP(TArray<FGASCoreAttributeInformation>& OutRows) const { OutRows = AttributeInformation; }

	// Existing helper is still fine if you keep tag-based lookups around
	virtual FGASCoreAttributeInformation FindAttributeInfoByTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

private:
	
	UPROPERTY(EditDefaultsOnly, meta=(TitleProperty="{AttributeName}"))
	TArray<FGASCoreAttributeInformation> AttributeInformation;
};
