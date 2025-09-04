// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "ClickToMove.h"                 // Defines the NAVIGATION trace channel macro used for cursor tracing
#include "Components/ActorComponent.h"
#include "ClickToMoveComponent.generated.h"

// Forward declarations to keep compile units light and avoid extra header includes here.
// We avoid including heavy headers in the .h to reduce compile times.
class USplineComponent;
class AController;
class APlayerController;
class APawn;

/**
 * UClickToMoveComponent
 *
 * High-level behavior
 * - Hold-to-move:
 *   While the left mouse button (LMB) is held, we steer the pawn directly toward the cursor hit location.
 *   The cursor hit (from an internal trace or an external provided hit) is projected onto the navmesh so
 *   clicking non-walkable geometry (e.g., static meshes) still produces a valid move target.
 *
 * - Short-press autorun:
 *   On LMB release, if the press duration is short (<= ShortPressThreshold), we build a path on the navmesh
 *   and then step through its points over time (autorun), driving AddMovementInput toward each point in sequence.
 *
 * Responsibilities and ownership
 * - This component is intended to be placed on a PlayerController (preferred) or a Pawn.
 *   It queries the "controlled pawn" via the owner/owner controller to issue movement input.
 *
 * Networking model
 * - All decisions and AddMovementInput calls are executed only for local PlayerControllers (client-side).
 *   CharacterMovement replicates the resulting movement to the server/other clients.
 *
 * Spline notes
 * - A USplineComponent is created via NewObject and intentionally not attached/registered.
 *   It is used purely for optional path visualization or math; there is no automatic scene participation.
 *
 * Design goals
 * - Minimize per-tick work: only tick when autorun is active (path is being followed).
 * - Separate responsibilities: clicking/holding establishes goals; autorun consumes cached path points.
 * - Robustness: project cursor hits onto navmesh so non-walkable clicks still produce a valid target.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CLICKTOMOVE_API UClickToMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Lightweight constructor; do not do heavy work here.
	// Use BeginPlay for initialization that depends on the world/owner being fully initialized.
	UClickToMoveComponent();

	// We enable ticking only during autorun; keep it disabled otherwise for performance.
	// TickComponent drives the per-frame autorun step when a path is active.
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== Input forwarding (call these from your PlayerController) =====

	// LMB pressed (start of a click): stop autorun and reset press timers/state.
	// This also clears any previously cached path so the next movement order starts fresh.
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
	void OnClickPressed();

	// LMB held (every frame while held):
	// - If bUseInternalHitResult is true, perform an internal cursor trace on the NAVIGATION channel.
	// - Otherwise, use the provided InHitResult (commonly from a highlight/interaction system).
	// In both cases, we project the point to the navmesh to ensure a reachable target.
	// While holding, movement is direct steering toward the projected cursor location (no path building).
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
	void OnClickHeld(bool bUseInternalHitResult, const FHitResult& InHitResult);

	// LMB released (end of click): if the press was short, build a nav path and start autorun.
	// If the press was long (held beyond ShortPressThreshold), we skip autorun (the pawn already moved).
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
	void OnClickReleased();

	// ===== Gates and external control =====

	// If the game is in a targeting/interaction mode (e.g., casting abilities on an actor),
	// set this so click-to-move yields and does not consume the click.
	// This allows sharing LMB between movement and interaction systems.
	UFUNCTION(BlueprintCallable, Category="ClickToMove")
	void SetIsTargeting(const bool bInTargeting) { bIsTargeting = bInTargeting; }

	// Optional external toggle for autorun (normally managed internally).
	// Can be used by higher-level systems to cancel or force autorun.
	UFUNCTION(BlueprintCallable, Category="ClickToMove")
	void SetAutoRunActive(const bool bInActive) { bIsAutoRunning = bInActive; }

	// Forcefully stop autorun and disable ticking (used by higher-level systems if needed).
	// Clears cached path state so a subsequent order starts from a clean slate.
	UFUNCTION(BlueprintCallable, Category="ClickToMove|Orders")
	void StopMovement();

protected:
	// Component lifecycle start:
	// - Create the optional spline helper (unattached/unregistered).
	// - Disable ticking; we only tick during autorun.
	// Avoid heavy work or gameplay logic here (no world time yet).
	virtual void BeginPlay() override;

	// Autorun step: called from Tick when bIsAutoRunning == true.
	// Follows path points sequentially to achieve accurate, corner-hugging motion.
	// Uses 2D distances to avoid false non-arrivals due to minor Z differences (stairs/ramps).
	void AutoRun();

	// On short-press release:
	// - Project the desired goal to the navmesh.
	// - Build a path to that goal.
	// - Cache the path points and enable autorun + ticking.
	// If no valid path is found, autorun is not started.
	void FindPathToLocation();

private:
	// ===== Internals =====

	// Create (if needed) the spline helper used for optional path visualization/math.
	// The spline is intentionally left unattached and unregistered to avoid scene overhead.
	// If you wish to visualize, you can optionally attach/register it at runtime elsewhere.
	USplineComponent* EnsureSplineNoAttach();

	// Context helpers (owner may be a Controller or a Pawn).
	// These helpers encapsulate the logic to find the controlling PlayerController and its pawn.
	AController* GetOwnerController() const;
	APlayerController* GetOwnerPC() const;
	APawn* GetControlledPawn() const;

	// Drive AddMovementInput toward a world destination (shared by held/internal/external traces).
	// This does not teleport; it simply feeds the CharacterMovement with an input vector for this frame.
	void ApplyMoveToward(const FVector& DestinationWorld) const;

	// Projects a world-space point to the nearest navigable location on the navmesh.
	// Returns false if no valid navmesh point is found within the configured extents.
	// Use this to convert arbitrary cursor hits (including non-walkable meshes) into reachable goals.
	bool ProjectPointToNavmesh(const FVector& InWorld, FVector& OutProjected) const;

private:
	// ===== Config (per-instance; reasonable defaults) =====

	// Maximum time for a click to be considered a "short press" (seconds).
	// Presses longer than this threshold are treated as hold-to-move (no autorun on release).
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.05"))
	float ShortPressThreshold = 0.5f;

	// Distance used to decide when we've "reached" a path point or the final destination (units).
	// Generally set >= pawn capsule radius to avoid orbiting around the target.
	// Consider tuning per pawn scale/speed. Larger values produce snappier advancement along the path.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="1.0"))
	float AcceptanceRadius = 50.f;

	// Optional: scale the acceptance radius by current movement speed to reduce overshoot at higher speeds.
	// EffectiveAcceptance = clamp(AcceptanceRadius + Speed2D * AcceptanceSpeedScale, AcceptanceRadiusMin, AcceptanceRadiusMax).
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
	bool bScaleAcceptanceBySpeed = true;

	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0"))
	float AcceptanceSpeedScale = 0.05f; // e.g., 600 uu/s -> +30 uu

	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0"))
	float AcceptanceRadiusMin = 30.f;

	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0"))
	float AcceptanceRadiusMax = 120.f;

	// Optional: look ahead toward the next path point to soften turns.
	// We use a simple blend between the current target and the next point.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
	bool bUseLookahead = true;

	// 0 = no lookahead (aim at current point only), 1 = fully aim at next point.
	// Typical values 0.2..0.5 gently round corners without cutting too much.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LookaheadBlendAlpha = 0.3f;

	// Trace channel used by the component for internal cursor traces while holding.
	// NAVIGATION comes from ClickToMove.h and should be configured to block walkable ground.
	// Ensure your pawn ignores this channel to avoid self-hits.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
	TEnumAsByte<ECollisionChannel> CursorTraceChannel = NAVIGATION;

	// Half-extents of the box used by ProjectPointToNavigation when searching for a nearby navmesh point.
	// Increase to snap across small gaps/steps; decrease to avoid jumping to unintended floors.
	// Z extent especially controls vertical reach (useful on stairs/ramps/multi-level spaces).
	UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
	FVector NavProjectExtent = FVector(200.f, 200.f, 200.f);

	// ===== Runtime State (Visible for debugging) =====

	// Latest cursor world point (while holding), or the final nav point (during autorun).
	// For autorun, this is typically set to the last point of the path for informational/debug purposes.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	FVector CachedDestination = FVector::ZeroVector;

	// Accumulated hold time since OnClickPressed (seconds).
	// Used to classify between hold-to-move vs short-press autorun.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State", meta=(ClampMin="0.0"))
	float FollowTime = 0.f;

	// True when following a built path (enables Tick to call AutoRun).
	// This gate ensures we don’t waste cycles when there’s no path to follow.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	bool bIsAutoRunning = false;

	// True when the current click is reserved for targeting/interactions (movement yields).
	// Set by the PlayerController based on interaction/highlight state.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	bool bIsTargeting = false;

	// Cached path points returned from the navmesh when we built a path for autorun.
	// Index 0 is the start (current pawn location); we begin traveling from index 1.
	// PathPoints are consumed by AutoRun to steer accurately from one corner to the next.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	TArray<FVector> PathPoints;

	// Current index into PathPoints array (the target point we are moving toward).
	// Initialized to 1 on a newly built path; INDEX_NONE indicates no active path.
	// AutoRun advances this index as points are reached.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
	int32 PathIndex = INDEX_NONE;

	// ===== Optional Helpers =====

	// Optional spline used to visualize the path (debug). Not attached/registered by default.
	// You can attach/register it manually if you want it to render in-game/editor.
	UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|Debug")
	TObjectPtr<USplineComponent> Spline = nullptr;

	// ===== Debug drawing for nav projection =====

	// Toggle debug drawing of the ProjectPointToNavmesh search box and the projected point.
	// Helpful when tuning NavProjectExtent so projections land where you expect.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Debug")
	bool bDebugProjectToNav = false;

	// Color for the debug search box drawn around the input point (InWorld).
	UPROPERTY(EditAnywhere, Category="ClickToMove|Debug")
	FColor DebugProjectBoxColor = FColor::Cyan;

	// Color for the projected result point on the navmesh.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Debug")
	FColor DebugProjectedPointColor = FColor::Green;

	// Lifetime (seconds) for debug lines/shapes.
	// Use small values to avoid clutter; set to 0 for single-frame rendering.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Debug", meta=(ClampMin="0.0"))
	float DebugDrawLifetime = 1.5f;

	// Line thickness when drawing debug shapes.
	UPROPERTY(EditAnywhere, Category="ClickToMove|Debug", meta=(ClampMin="0.1"))
	float DebugLineThickness = 1.5f;
};