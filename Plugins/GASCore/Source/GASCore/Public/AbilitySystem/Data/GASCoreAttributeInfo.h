// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "Engine/DataAsset.h"
#include "GASCoreAttributeInfo.generated.h"

class UTexture2D;

/**
 * FGASCoreAttributeInfo
 *
 * Core data structure for attribute metadata in the GASCore plugin.
 * Contains static metadata (names, descriptions, icons) and runtime binding information.
 */
USTRUCT(BlueprintType)
struct GASCORE_API FGASCoreAttributeInfo
{
	GENERATED_BODY()

	/** Unique tag identifying this attribute (e.g., Attributes.Primary.Strength) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	FGameplayTag AttributeTag = FGameplayTag();

	/** Localized display name for the attribute */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	FText AttributeName = FText();

	/** Localized description explaining what this attribute does */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	FText AttributeDescription = FText();

	/** Current runtime value (populated by controller, not stored in data asset) */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Info")
	float AttributeValue = 0.0f;

	/** Optional format string for displaying the value (e.g., "{0}%" for percentages) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	FText ValueFormat = FText();

	/** Optional icon for UI representation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	TObjectPtr<UTexture2D> AttributeIcon = nullptr;

	/** Whether this is a primary attribute (affects UI grouping) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	bool bIsPrimary = false;

	/** FGameplayAttribute getter for reading current value from AttributeSet */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	FGameplayAttribute AttributeGetter;

	/** Default constructor with safe defaults */
	FGASCoreAttributeInfo()
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
 * UGASCoreAttributeInfo
 *
 * Data Asset base class for storing attribute metadata collections.
 * Provides zero-copy access to attribute information and efficient lookup functions.
 */
UCLASS(BlueprintType, Blueprintable)
class GASCORE_API UGASCoreAttributeInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 
	 * Zero-copy accessor for the attribute information array
	 * @return Const reference to the attribute information array
	 */
	UFUNCTION(BlueprintCallable, Category = "Attribute Info")
	const TArray<FGASCoreAttributeInfo>& GetAttributeInformation() const { return AttributeInformation; }

	/** 
	 * Find attribute info by GameplayTag with optional logging
	 * @param AttributeTag - The tag to search for
	 * @param bLogNotFound - Whether to log warning if tag not found
	 * @return Const reference to the corresponding attribute info, or reference to default static instance if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Attribute Info")
	const FGASCoreAttributeInfo& FindAttributeInfoByTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

	/** Get all primary attributes (bIsPrimary == true) */
	UFUNCTION(BlueprintCallable, Category = "Attribute Info")
	TArray<FGASCoreAttributeInfo> GetPrimaryAttributes() const;

	/** Get all secondary attributes (bIsPrimary == false) */
	UFUNCTION(BlueprintCallable, Category = "Attribute Info")
	TArray<FGASCoreAttributeInfo> GetSecondaryAttributes() const;

protected:
	/** Array of all attribute information entries */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Info")
	TArray<FGASCoreAttributeInfo> AttributeInformation;

private:
	/** Static default instance returned when attribute not found */
	static const FGASCoreAttributeInfo DefaultAttributeInfo;
};