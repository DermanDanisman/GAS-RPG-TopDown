// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "AbilitySystem/Data/GASCoreAttributeInfo.h"
#include "Engine/DataAsset.h"
#include "AttributeInfo.generated.h"

class UTexture2D;

USTRUCT(Blueprintable)
struct FTDAttributeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();

	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.0f;

	/** Optional format string for displaying the value (e.g., "{0}%" for percentages) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ValueFormat = FText();

	/** Optional icon for UI representation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> AttributeIcon = nullptr;

	/** Whether this is a primary attribute (affects UI grouping) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsPrimary = false;

	/** FGameplayAttribute getter for reading current value from AttributeSet */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute AttributeGetter;

	/** Default constructor with safe defaults */
	FTDAttributeInfo()
	{
		AttributeTag = FGameplayTag();
		AttributeName = FText::GetEmpty();
		AttributeDescription = FText::GetEmpty();
		AttributeValue = 0.0f;
		ValueFormat = FText::FromString(TEXT("{0}"));
		AttributeIcon = nullptr;
		bIsPrimary = false;
		AttributeGetter = FGameplayAttribute();
	}
};

/**
 * UAttributeInfo
 * 
 * Game-specific data asset that extends GASCore functionality.
 * Stores TD-specific attribute metadata and provides compatibility with existing systems.
 */
UCLASS()
class RPG_TOPDOWN_API UAttributeInfo : public UGASCoreAttributeInfo
{
	GENERATED_BODY()

public:
	/** 
	 * Find attribute info by GameplayTag (TD-specific version)
	 * @param AttributeTag - The tag to search for
	 * @param bLogNotFound - Whether to log warning if tag not found
	 * @return TD-specific attribute info, or default if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Attribute Info")
	FTDAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

	/** 
	 * Zero-copy accessor for TD attribute information array
	 * @return Const reference to the TD attribute information array
	 */
	UFUNCTION(BlueprintCallable, Category = "Attribute Info")
	const TArray<FTDAttributeInfo>& GetAttributeInformation() const { return AttributeInfos; }

protected:
	/** Array of TD-specific attribute information entries */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FTDAttributeInfo> AttributeInfos;

private:
	/** Static default instance returned when attribute not found */
	static const FTDAttributeInfo DefaultTDAttributeInfo;
};
