// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

// Summary:
//   Base UObject for all GAS-oriented widget controllers.
//   It centralizes references to gameplay systems (PlayerController, PlayerState, ASC, AttributeSet)
//   and provides a consistent lifecycle for UI initialization and reactive updates.
//
// Pattern (UI MVC):
//   - Model: GAS (AbilitySystemComponent + AttributeSet)
//   - Controller: UCoreWidgetController (reads model, broadcasts to UI)
//   - View: UUserWidget-derived widgets (bind to controller delegates, display only)
//
// Responsibilities of derived controllers:
//   - Override BroadcastInitialValues() to push current attribute values (e.g., health/mana) on init
//   - Override BindCallbacksToDependencies() to subscribe to ASC/AttributeSet delegates and rebroadcast
//   - Optionally emit UI messages via MessageWidgetRowDelegate when model events occur
//
// Usage:
//   1) Construct controller (C++/Blueprint).
//   2) Call SetWidgetControllerParams() with valid references (PC, PS, ASC, AttributeSet).
//   3) In your widget (e.g., UAuraUserWidget-like), call SetWidgetController(this controller),
//      then trigger WidgetControllerSet (BP event) to bind UI handlers.
//   4) After bindings, call BroadcastInitialValues() once to initialize UI state.
//
// Lifetime & Safety Notes:
//   - Ensure AbilitySystemComponent and AttributeSet are valid before binding in derived classes.
//   - If using AddLambda with [this] captures in derived controllers, ensure the controller outlives bindings
//     or use weak captures/handles to avoid dangling references.
//   - MessageWidgetRowDelegate broadcasts whole rows; keep row structs lightweight.
//
// Related types:
//   - FUIMessageWidgetRow: DataTable row mapping a GameplayTag to message content/widget/icon.
//   - UCoreAbilitySystemComponent: can broadcast effect asset tags to drive UI messages.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "Engine/DataTable.h" // FTableRowBase
#include "CoreWidgetController.generated.h"

class UCoreUserWidget;
class UTexture2D;
class UAttributeSet;
class UAbilitySystemComponent;

/**
 * FWidgetControllerParams
 *
 * Struct for passing references to all systems a GAS widget controller needs.
 * - BlueprintType: designers can initialize controllers from Blueprints.
 * - Members are TObjectPtr<> for GC awareness.
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	/** Default constructor: initializes pointers to nullptr (safe default). */
	FWidgetControllerParams() = default;

	/**
	 * Parameterized constructor for initializing all system references at once.
	 * @param InPlayerController         Owner player controller (HUD/input).
	 * @param InPlayerState              Persistent/replicated player state.
	 * @param InAbilitySystemComponent   GAS component (abilities/effects/tags).
	 * @param InAttributeSet             Attributes (health, mana, etc.).
	 */
	explicit FWidgetControllerParams(APlayerController* InPlayerController,
	                                 APlayerState* InPlayerState,
	                                 UAbilitySystemComponent* InAbilitySystemComponent,
	                                 UAttributeSet* InAttributeSet)
	    : PlayerController(InPlayerController)
	    , PlayerState(InPlayerState)
	    , AbilitySystemComponent(InAbilitySystemComponent)
	    , AttributeSet(InAttributeSet)
	{}

	/** Owner player controller; may be null until possession/creation completes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASCore|WidgetController")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** Player state for identity/score/persistent data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASCore|WidgetController")
	TObjectPtr<APlayerState> PlayerState = nullptr;

	/** GAS component for abilities, effects, and gameplay tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASCore|WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	/** Attribute set instance holding gameplay-modifiable stats. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASCore|WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/**
 * UCoreWidgetController
 *
 * Base class for GAS widget controllers. Manages references to gameplay systems
 * and defines a standard initialization contract for derived controllers.
 */
UCLASS()
class GASCORE_API UCoreWidgetController : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Initializes all references at once from a single struct.
	 * Call immediately after constructing the controller and before any UI binding.
	 * Safe to call multiple times to refresh references (e.g., on pawn possession changes).
	 */
	UFUNCTION(BlueprintCallable, Category = "GASCore|WidgetController")
	void SetWidgetControllerParams(const FWidgetControllerParams& InWidgetControllerParams);

	/**
	 * Broadcast the initial values of attributes to the UI.
	 * Derived classes should override and push current values (e.g., health/mana)
	 * after widgets have bound to the controller's delegates.
	 */
	virtual void BroadcastInitialValues();

	/**
	 * Bind attribute/effect change callbacks to the GAS systems.
	 * Derived classes should override and subscribe to ASC/AttributeSet delegates
	 * to forward updates to widgets via controller delegates.
	 */
	virtual void BindCallbacksToDependencies();

protected:
	/** Owning player controller (HUD/input). */
	UPROPERTY(BlueprintReadOnly, Category = "GASCore|Widget Controller")
	TObjectPtr<APlayerController> PlayerController;

	/** Owning player state (persistent/replicated player info). */
	UPROPERTY(BlueprintReadOnly, Category = "GASCore|Widget Controller")
	TObjectPtr<APlayerState> PlayerState;

	/** Ability System Component (GAS). */
	UPROPERTY(BlueprintReadOnly, Category = "GASCore|Widget Controller")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Attribute Set with gameplay stats. */
	UPROPERTY(BlueprintReadOnly, Category = "GASCore|Widget Controller")
	TObjectPtr<UAttributeSet> AttributeSet;
};