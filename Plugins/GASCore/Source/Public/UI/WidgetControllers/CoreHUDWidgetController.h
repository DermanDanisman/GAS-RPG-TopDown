// Copyright:
// © 2025 Heathrow (Derman). All rights reserved.
//
// Purpose:
// - Defines a Widget Controller that sits between the GAS data model (ASC + AttributeSet)
//   and your UUserWidget-based HUD. It follows an MVC-style pattern:
//   - Model: GAS (ASC, AttributeSet, GameplayEffects, GameplayTags)
//   - Controller: this class (reads from Model, broadcasts to View)
//   - View: Widgets (bind to controller delegates; display-only)
//
// Responsibilities:
// - Broadcast initial attribute values to the UI when the controller is set up
// - Subscribe to GAS attribute change delegates and rebroadcast values to widgets
// - Relay GameplayEffect asset tag events (via ASC) to the UI (e.g., messages/toasts)
// - Optionally look up UI message rows by gameplay tag in a DataTable
//
// Notes on lifetime and safety:
// - Bind callbacks only after AbilitySystemComponent and AttributeSet are valid and initialized
// - Lambdas capture `this`; ensure the controller outlives bindings or use weak captures
// - DataTable lookups assume the row key is the tag's FName (Tag.GetTagName())
//
// See also:
// - UI/WidgetControllers/CoreHUDWidgetController.cpp for implementation details
// - Components/CoreAbilitySystemComponent.* for the Effect Asset Tags delegate

#pragma once

#include "CoreMinimal.h"
#include "CoreWidgetController.h"
#include "CoreHUDWidgetController.generated.h"

/**
 * FUIMessageWidgetRow
 *
 * Row type for UI message DataTables.
 * A GameplayTag addresses a message payload
 * (localized text, a widget class, and an optional image) to drive notifications/toasts.
 *
 * Typical usage:
 * - In a controller, listen for "UI.Message.*" tags from the ASC (e.g., effect asset tags).
 * - Look up the row by tag (row name == tag's FName).
 * - Broadcast row via MessageWidgetRowDelegate for the HUD to render.
 */
USTRUCT(BlueprintType)
struct FUIMessageWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The unique tag for this message (e.g., UI.Message.HealthPotion). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Message")
	FGameplayTag MessageTag;

	/** Localized user-facing message text. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Message")
	FText MessageText;

	/** Optional widget class to render the message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Message")
	TSubclassOf<UCoreUserWidget> MessageWidget;

	/** Optional icon displayed with the message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Message")
	TObjectPtr<UTexture2D> MessageImage;

	FUIMessageWidgetRow()
	{
		MessageTag = FGameplayTag();
		MessageText = FText();
		MessageWidget = nullptr;
		MessageImage = nullptr;
	}
};

/** Broadcasts a message widget row to the UI (e.g., HUD overlay). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIMessageWidgetRowSignature, FUIMessageWidgetRow, MessageWidgetRow);

// Declare multicast delegates for different HUD attribute changes.
// These are BlueprintAssignable so widgets can bind in BP to receive updates.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/**
 * UCoreHUDWidgetController
 *
 * Bridges the GAS data model to HUD widgets:
 * - On setup, broadcasts initial attribute values so widgets can initialize their displays
 * - Subscribes to attribute change delegates, to push real-time updates
 * - Listens for GameplayEffect asset tags (from ASC) and forwards matching UI message rows
 */
UCLASS()
class GASCORE_API UCoreHUDWidgetController : public UCoreWidgetController
{
	GENERATED_BODY()

public:

	/**
	 * Broadcast initial values to the UI.
	 * Called once the controller has valid references (PlayerController, PlayerState, ASC, AttributeSet).
	 * Override from base to push initial attribute values to widgets.
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * Subscribe to attribute/value change delegates on the AbilitySystemComponent.
	 * Override from base to bind your attribute delegates and any custom ASC delegates.
	 */
	virtual void BindCallbacksToDependencies() override;
	
	/** Delegate for listening to Health value changes (NewHealth). */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	/** Delegate for listening to MaxHealth changes (NewMaxHealth). */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	/** Delegate for listening to Mana value changes (NewMana). */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	/** Delegate for listening to MaxMana changes (NewMaxMana). */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;

	/** Delegate for listening to Stamina value changes (NewStamina). */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnStaminaChanged;

	/** Delegate for listening to MaxStamina changes (NewMaxStamina). */
	UPROPERTY(BlueprintAssignable, Category="HUD Widget Controller|Attributes")
	FOnAttributeChangedSignature OnMaxStaminaChanged;

	/**
	 * Generic message dispatch to the UI.
	 * Controllers can broadcast a row (looked up from a DataTable) to request
	 * the HUD render a message/notification.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Core|Widget Controller|UI")
	FUIMessageWidgetRowSignature MessageWidgetRowDelegate;

protected:

	/**
	 * Optional DataTable that maps GameplayTags (row names) to UI message rows.
	 * - Expected row key: Tag.GetTagName() (e.g., "UI.Message.HealthPotion")
	 * - Expected struct type: FUIMessageWidgetRow (custom USTRUCT with MessageTag and display data)
	 */
	UPROPERTY(EditDefaultsOnly, Category="HUD Widget Controller|UI")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	/**
	 * Utility to fetch a DataTable row by gameplay tag.
	 * - Template T is the row UStruct type (e.g., FUIMessageWidgetRow)
	 * - Returns a pointer to the row or nullptr if not found
	 */
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
};

template <typename T>
T* UCoreHUDWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	// Note: This assumes the row name equals the tag's FName (e.g., "UI.Message.HealthPotion")
	// You may want to guard against DataTable == nullptr and return nullptr for safety.
	return DataTable ? DataTable->FindRow<T>(Tag.GetTagName(), TEXT("UCoreHUDWidgetController::GetDataTableRowByTag")) : nullptr;
}