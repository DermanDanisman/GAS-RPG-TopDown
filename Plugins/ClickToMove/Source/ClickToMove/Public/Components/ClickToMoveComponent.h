// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClickToMoveComponent.generated.h"

// Forward declarations to keep compile units light and avoid extra headers here.
class USplineComponent;
class AController;
class APlayerController;
class APawn;

/**
 * UClickToMoveComponent
 *
 * Purpose:
 * - Click-To-Move behavior with two modes:
 *   - Hold LMB: steer the pawn directly toward the cursor.
 *   - Short press (on release): build a nav path and autorun along a spline.
 *
 * Ownership:
 * - Add to a PlayerController (preferred) or a Pawn. Component looks up the controlled Pawn to drive movement.
 *
 * Networking:
 * - Executes only on the local controller. Movement replication is handled by CharacterMovement.
 *
 * Notes:
 * - Spline is created with NewObject, left unattached/unregistered (no scene/render overhead). Used as a path helper.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CLICKTOMOVE_API UClickToMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Constructor: keep lightweight. No heavy work here.
	UClickToMoveComponent();

	// Tick only used during autorun; otherwise disabled.
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== Input forwarding (call from your controller) =====

	// LMB pressed: stop autorun, reset timers.
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
	void OnClickPressed();

	// LMB held: either perform internal cursor trace (bUseInternalHitResult=true) or use a provided hit result.
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
	void OnClickHeld(bool bUseInternalHitResult, const FHitResult& InHitResult);

	// LMB released: if it was a short press, build a nav path and start autorun.
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
	void OnClickReleased();

	// ===== Gates and external control =====

	// Gate movement when click is intended for targeting/interactions.
	UFUNCTION(BlueprintCallable, Category="ClickToMove")
	void SetIsTargeting(const bool bInTargeting) { bIsTargeting = bInTargeting; }

	// Optional external toggle for autorun (normal flow toggles this internally).
	UFUNCTION(BlueprintCallable, Category="ClickToMove")
	void SetAutoRunActive(const bool bInActive) { bIsAutoRunning = bInActive; }

	// Stop any ongoing autorun and disable ticking.
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Orders")
	void StopMovement();

protected:
	// Initialize and create optional spline helper; disable tick until autorun starts.
	virtual void BeginPlay() override;

	// Step along the spline during autorun (called from Tick when bIsAutoRunning is true).
	void AutoRun();

	// Build a path and spline on short press release; enable autorun.
	void FindPathToLocation();

private:
	// ===== Internals =====

	// Create spline helper (unattached/unregistered), if needed.
	USplineComponent* EnsureSplineNoAttach();

	// Resolve controller/pawn context regardless of whether Owner is a Controller or a Pawn.
	AController* GetOwnerController() const;
	APlayerController* GetOwnerPC() const;
	APawn* GetControlledPawn() const;

	// Apply AddMovementInput toward a world destination (shared by held/internal/external traces).
	void ApplyMoveToward(const FVector& DestinationWorld) const;

private:
	// ===== Config (per-instance; reasonable defaults) =====

	// Short press time window in seconds to decide autorun vs hold-to-move.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.05"))
	float ShortPressThreshold = 0.5f;

	// Distance considered "arrived" at final destination during autorun.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="1.0"))
	float AcceptanceRadius = 50.f;

	// Collision channel for cursor traces when using internal tracing.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
	TEnumAsByte<ECollisionChannel> CursorTraceChannel = ECC_Visibility;

	// ===== Runtime State (Visible for debugging) =====

	// Latest cursor world point (during hold) or final path endpoint (during autorun).
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	FVector CachedDestination = FVector::ZeroVector;

	// Accumulated hold time since OnClickPressed (seconds).
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State", meta=(ClampMin="0.0"))
	float FollowTime = 0.f;

	// True if following a built path (tick enabled).
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	bool bIsAutoRunning = false;

	// True if current click is intended for targeting/interactions (movement yields).
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	bool bIsTargeting = false;

	// ===== Optional Helpers =====

	// Unregistered/unattached spline to hold/compute a path (no scene overhead).
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|Debug")
	TObjectPtr<USplineComponent> Spline = nullptr;
};