// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Engine Includes =====
#include "CoreMinimal.h"
#include "AttributeSet.h"           // For FGameplayAttribute used in each row
#include "GameplayTagContainer.h"   // For FGameplayTag used in each row
#include "Engine/DataAsset.h"

#include "GASCoreAttributeInfoDataAsset.generated.h"

// ===== Row type authored in the Data Asset =====

/**
 * FGASCoreAttributeInformation
 *
 * Author a single UI row for an attribute, including:
 * - AttributeTag: stable identity used by UI and logic.
 * - AttributeName/AttributeDescription: localized UI text.
 * - AttributeGetter: the FGameplayAttribute identity to query live numeric values from an AttributeSet.
 * - AttributeValue: runtime-filled numeric value before broadcasting to UI.
 */
USTRUCT(BlueprintType)
struct FGASCoreAttributeInformation
{
	GENERATED_BODY()

	/** Stable identity for this attribute row; used by widgets to filter updates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute Info")
	FGameplayTag AttributeTag;

	/** Localized display name for UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute Info")
	FText AttributeName = FText();

	/** Localized description tooltip or extended text for UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute Info")
	FText AttributeDescription = FText();

	/** Runtime-filled numeric value (computed right before broadcasting to UI). */
	UPROPERTY(BlueprintReadOnly, Category="Attribute Info")
	float AttributeValue = 0.f;

	/**
	 * Attribute identity (FGameplayAttribute) selected in the editor.
	 * Used to:
	 * - Bind ASC value change delegates (live updates).
	 * - Resolve the current numeric value via AttributeGetter.GetNumericValue(AttributeSet).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute Info")
	FGameplayAttribute AttributeGetter;
};

// ===== Data Asset that aggregates rows =====

/**
 * UGASCoreAttributeInfoDataAsset
 *
 * Designer-authored container of attribute UI rows.
 * - The controller iterates these rows to broadcast initial values.
 * - It also uses the FGameplayAttribute identity per row to bind live updates.
 */
UCLASS(BlueprintType)
class GASCORE_API UGASCoreAttributeInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * Return all authored rows.
	 * Note: Returns by value for BP convenience. For large sets, consider adding a C++ const-ref accessor.
	 */
	TArray<FGASCoreAttributeInformation> GetAttributeInformation() const { return AttributeInformation; }

	/** BP-friendly way to retrieve the list without exposing the UPROPERTY directly. */
	UFUNCTION(BlueprintPure, Category="GASCore|Attribute Info")
	void GetAttributeInformation_BP(TArray<FGASCoreAttributeInformation>& OutRows) const { OutRows = AttributeInformation; }

	/**
	 * Find the first row that matches AttributeTag exactly.
	 * @param AttributeTag		Tag to search for (MatchesTagExact).
	 * @param bLogNotFound		Optional error logging if the tag is missing from this asset.
	 * @return					Matching row by value, or a default-constructed row if not found.
	 *
	 * Note: Returns by value (convenient for BP). For C++ hot paths, you may add a pointer/const-ref accessor.
	 */
	virtual FGASCoreAttributeInformation FindAttributeInfoByTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

private:

	/** Authored rows. TitleProperty helps identify each row in the editor details panel. */
	UPROPERTY(EditDefaultsOnly, Category="GASCore|Attribute Info", meta=(TitleProperty="{AttributeName}"))
	TArray<FGASCoreAttributeInformation> AttributeInformation;
};