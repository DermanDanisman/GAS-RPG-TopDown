// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Components/ClickToMoveComponent.h"

#include "NavigationPath.h"                // UNavigationPath: container for path points computed by the nav system
#include "NavigationSystem.h"              // UNavigationSystemV1: entry point for navigation queries/projection
#include "Components/SplineComponent.h"    // USplineComponent: optional helper for path visualization/math
#include "DrawDebugHelpers.h"              // Debug draw primitives (spheres/lines/boxes)
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

// Constructor: allow ticking, but we enable it only during autorun.
// This keeps idle cost low. CharacterMovement handles actual physics/motion.
UClickToMoveComponent::UClickToMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false); // client-driven; CharacterMovement replicates. We only run logic on local PC.
}

void UClickToMoveComponent::BeginPlay()
{
	Super::BeginPlay();
	
	EnsureSplineNoAttach(); // Create unattached/unregistered spline for optional visualization. No scene cost.

	// Start with ticking disabled; autorun flow will enable as needed to avoid per-frame overhead when idle.
	SetComponentTickEnabled(false);
}

void UClickToMoveComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Only drive autorun path following when active.
	// This gate is critical to ensure Tick does nothing when there is no path to follow.
	if (bIsAutoRunning)
	{
		AutoRun();
	}
}

void UClickToMoveComponent::OnClickPressed()
{
	// Reset hold state timer (used to distinguish short vs long press on release).
	// Also stop any ongoing autorun so holding starts a new movement order.
	SetAutoRunActive(false);
	FollowTime = 0.f;
	SetComponentTickEnabled(false);

	// Clear any previous path to ensure we don't reuse stale points.
	PathPoints.Reset();
	PathIndex = INDEX_NONE;
}

void UClickToMoveComponent::OnClickHeld(const bool bUseInternalHitResult, const FHitResult& InHitResult)
{
	// Local-only guard: only the local PlayerController should drive movement.
	// Prevents server or remote clients from issuing AddMovementInput here.
	const APlayerController* PC = GetOwnerPC();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// If we're targeting, do not consume movement input.
	// Allows LMB to be reused for interactions/abilities without moving.
	if (bIsTargeting)
	{
		return;
	}

	// Accumulate hold time (used on release to decide short-press autorun vs hold-to-move).
	if (const UWorld* World = GetWorld())
	{
		FollowTime += World->GetDeltaSeconds();
	}

	// Either do an internal trace or use the provided hit result from elsewhere (e.g., HighlightInteraction).
	// We prefer to minimize tracing, but when we do, we use a dedicated NAVIGATION channel for walkable surfaces.
	FVector RawHitPoint = CachedDestination; // default to last known destination in case we fail a trace
	bool bHaveBlockingHit = false;

	if (bUseInternalHitResult)
	{
		// Internal cursor trace under the mouse using the configured channel.
		// bTraceComplex=false for performance; switch to true only if you require per-triangle hits.
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(CursorTraceChannel, /*bTraceComplex=*/false, HitResult))
		{
			RawHitPoint = HitResult.ImpactPoint;
			bHaveBlockingHit = true;
		}
	}
	else if (InHitResult.bBlockingHit)
	{
		// External hit provided (e.g., from a highlight system using its own channel).
		// We still project to navmesh below to ensure reachability.
		RawHitPoint = InHitResult.ImpactPoint;
		bHaveBlockingHit = true;
	}

	// Project to navmesh so clicks on static meshes still produce a valid move goal.
	// This converts arbitrary surface hits to a nearby navigable location (within NavProjectExtent).
	if (bHaveBlockingHit)
	{
		FVector Projected;
		if (ProjectPointToNavmesh(RawHitPoint, Projected))
		{
			CachedDestination = Projected; // store the latest navigable location under the cursor
			ApplyMoveToward(CachedDestination); // immediate steering while holding (no path building)
		}
	}
}

void UClickToMoveComponent::OnClickReleased()
{
	// Short-press: attempt to build a path and start autorun (path following).
	// Long-press: hold-to-move already handled movement; FindPathToLocation will early out.
	FindPathToLocation();
}

USplineComponent* UClickToMoveComponent::EnsureSplineNoAttach()
{
	if (!Spline)
	{
		// Owner as Outer ties lifetimes together; left unattached/unregistered by design.
		// Unregistered means it will not render nor tick as a scene component, which is desired here.
		Spline = NewObject<USplineComponent>(GetOwner(), TEXT("ClickToMoveSpline"));
	}
	return Spline;
}

AController* UClickToMoveComponent::GetOwnerController() const
{
	// If Owner is a Controller, return it directly.
	if (AController* AsController = Cast<AController>(GetOwner()))
	{
		return AsController;
	}
	// If Owner is a Pawn, return its Controller.
	if (const APawn* AsPawn = Cast<APawn>(GetOwner()))
	{
		return AsPawn->GetController();
	}
	// Otherwise, no controlling context found.
	return nullptr;
}

APlayerController* UClickToMoveComponent::GetOwnerPC() const
{
	// Convenience cast used for local-only checks and input tracing.
	return Cast<APlayerController>(GetOwnerController());
}

APawn* UClickToMoveComponent::GetControlledPawn() const
{
	// If Owner is a Pawn, return it.
	if (const APawn* AsPawnOwner = Cast<APawn>(GetOwner()))
	{
		return const_cast<APawn*>(AsPawnOwner);
	}
	// Otherwise, try the Controller's Pawn.
	if (const AController* C = GetOwnerController())
	{
		return C->GetPawn();
	}
	return nullptr;
}

void UClickToMoveComponent::StopMovement()
{
	// Disable autorun mode and ticking to avoid per-frame work.
	SetAutoRunActive(false);
	SetComponentTickEnabled(false);

	// Clear autorun state so subsequent orders start cleanly.
	PathPoints.Reset();
	PathIndex = INDEX_NONE;
}

void UClickToMoveComponent::ApplyMoveToward(const FVector& DestinationWorld) const
{
	// Local-only guard: never drive movement on non-local controllers.
	const APlayerController* PC = GetOwnerPC();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// Drive movement toward the given destination via AddMovementInput (CharacterMovement will handle motion).
	if (APawn* Pawn = GetControlledPawn())
	{
		// Use 3D direction; if your game is strictly planar, consider GetSafeNormal2D().
		const FVector Direction = (DestinationWorld - Pawn->GetActorLocation()).GetSafeNormal();
		if (!Direction.IsNearlyZero())
		{
			Pawn->AddMovementInput(Direction, 1.f);
		}
	}
}

bool UClickToMoveComponent::ProjectPointToNavmesh(const FVector& InWorld, FVector& OutProjected) const
{
	// Get the current world; during shutdown or very early lifecycle this may be null.
	const UWorld* World = GetWorld();
	if (!World) return false;

	// Retrieve the nav system for this world; returns null if nav is disabled or no nav data is present.
	if (const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
	{
		// Visualize the search volume (optional): a box centered at InWorld with half-extents NavProjectExtent.
		// This helps understand where the projection is allowed to search for a navigable point.
		if (bDebugProjectToNav)
		{
			DrawDebugBox(
				GetWorld(),
				InWorld,                  // center of the search box (your clicked/world point)
				NavProjectExtent,        // half-size (matches ProjectPointToNavigation extents)
				FQuat::Identity,         // no rotation (axis-aligned)
				DebugProjectBoxColor,    // color (configurable)
				/*bPersistentLines=*/false,
				/*LifeTime=*/DebugDrawLifetime,
				/*DepthPriority=*/0,
				/*Thickness=*/DebugLineThickness
			);
		}
		
		// Attempt to find the nearest point on the navmesh within the search extents.
		FNavLocation NavLoc;
		const bool bProjected = NavSys->ProjectPointToNavigation(
			InWorld,
			NavLoc,
			NavProjectExtent,           // half-extents of the search box around InWorld
			nullptr                     // NavData (use default/main)
		);
		if (bProjected)
		{
			OutProjected = NavLoc.Location; // Successfully found a reachable nav position.
			
			if (bDebugProjectToNav)
			{
				// Visualize the projected point and the vector from the input to the projected result.
				DrawDebugSphere(
					GetWorld(),
					OutProjected,
					/*Radius=*/12.f,
					/*Segments=*/12,
					DebugProjectedPointColor,
					/*bPersistentLines=*/false,
					/*LifeTime=*/DebugDrawLifetime,
					/*DepthPriority=*/0,
					/*Thickness=*/DebugLineThickness
				);

				DrawDebugLine(
					GetWorld(),
					InWorld,
					OutProjected,
					DebugProjectedPointColor,
					/*bPersistentLines=*/false,
					/*LifeTime=*/DebugDrawLifetime,
					/*DepthPriority=*/0,
					/*Thickness=*/DebugLineThickness
				);
			}
			
			return true;
		}
	}
	// No world, no nav system, or projection failed within the extents.
	return false;
}

void UClickToMoveComponent::AutoRun()
{
	// Local-only guard to avoid feeding inputs on non-local controllers.
	const APlayerController* PC = GetOwnerPC();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	APawn* Pawn = GetControlledPawn();
	// Stop cleanly if context is invalid (e.g., pawn destroyed or spline missing).
	if (!Pawn) { StopMovement(); return; }

	// Follow nav path points sequentially for accuracy.
	// We require at least two points (start + one target), a valid PathIndex, and that the index is within bounds.
	if (PathPoints.Num() >= 2 && PathIndex != INDEX_NONE && PathIndex < PathPoints.Num())
	{
		// Current pawn position and the current target path point.
		const FVector PawnLoc = Pawn->GetActorLocation();

		// Compute effective acceptance radius:
		// Base AcceptanceRadius, optionally scaled by current 2D speed (clamped to min/max).
		float EffectiveAcceptance = AcceptanceRadius;
		if (bScaleAcceptanceBySpeed)
		{
			const float Speed2D = Pawn->GetVelocity().Size2D(); // uu/s
			EffectiveAcceptance = FMath::Clamp(
				AcceptanceRadius + Speed2D * AcceptanceSpeedScale,
				AcceptanceRadiusMin,
				AcceptanceRadiusMax
			);
		}
		
		const FVector CurrentTarget = PathPoints[PathIndex];

		// Arrival/advance check:
		// When close enough to the current target (2D distance ignores height discrepancies), step to the next point.
		if (FVector::DistSquared2D(PawnLoc, CurrentTarget) <= FMath::Square(EffectiveAcceptance))
		{
			++PathIndex;

			// If we've consumed all points, we reached the end of the path; stop autorun.
			if (PathIndex >= PathPoints.Num())
			{
				StopMovement();
				return;
			}
		}

		// Aim point with optional lookahead toward the next path point (soften turns).
		// We blend between the current target and the next point to "round the corner".
		FVector AimPoint = PathPoints[PathIndex];
		if (bUseLookahead && (PathIndex + 1) < PathPoints.Num())
		{
			const FVector NextPoint = PathPoints[PathIndex + 1];
			AimPoint = FMath::Lerp(AimPoint, NextPoint, LookaheadBlendAlpha);
		}

		// Debug visualization (only in non-shipping/test builds)
		#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				{
					constexpr float DebugLifetime = 0.06f; // short-lived, refreshed each tick

					// 1) Acceptance circle at current target in the XY plane.
					// DrawDebugCircle signature: (World, Center, Radius, Segments, Color, bPersistent, LifeTime, Depth, Thickness, YAxis, ZAxis, bDrawAxis)
					// To draw in XY plane, use YAxis = X axis (1,0,0), ZAxis = Y axis (0,1,0).
					DrawDebugCircle(
						GetWorld(),
						CurrentTarget,
						/*Radius=*/EffectiveAcceptance,
						/*Segments=*/32,
						/*Color=*/FColor::Green,
						/*bPersistentLines=*/false,
						/*LifeTime=*/DebugLifetime,
						/*DepthPriority=*/0,
						/*Thickness=*/1.5f,
						/*YAxis=*/FVector(1.f, 0.f, 0.f),
						/*ZAxis=*/FVector(0.f, 1.f, 0.f),
						/*bDrawAxis=*/false
					);

					// 2) Markers: current target (yellow), next target (orange), aim point (cyan)
					DrawDebugSphere(GetWorld(), CurrentTarget, 10.f, 8, FColor::Yellow, false, DebugLifetime);
					if (PathIndex + 1 < PathPoints.Num())
					{
						DrawDebugSphere(GetWorld(), PathPoints[PathIndex + 1], 10.f, 8, FColor::Orange, false, DebugLifetime);
					}
					DrawDebugSphere(GetWorld(), AimPoint, 10.f, 8, FColor::Cyan, false, DebugLifetime);

					// 3) Aim line from pawn to aim point
					DrawDebugLine(GetWorld(), PawnLoc, AimPoint, FColor::Cyan, false, DebugLifetime, 0, 2.0f);

					// 4) On-screen readout (helpful to verify scaling and lookahead live)
					if (GEngine)
					{
						const float Speed2D = Pawn->GetVelocity().Size2D();
						GEngine->AddOnScreenDebugMessage(
							/*Key=*/42, /*Time=*/0.f, FColor::Yellow,
							FString::Printf(TEXT("Idx %d/%d  Speed2D=%.1f  EffAcc=%.1f  Lookahead=%s a=%.2f"),
								PathIndex, PathPoints.Num(),
								Speed2D, EffectiveAcceptance,
								bUseLookahead ? TEXT("ON") : TEXT("OFF"),
								LookaheadBlendAlpha)
						);
					}
				}
		#endif
		
		// Steer directly toward the aim point (2D vector keeps top-down motion planar).
		const FVector Direction = (AimPoint - PawnLoc).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			Pawn->AddMovementInput(Direction, 1.f);
		}
		return;

		// Note:
		// - Spline remains available for visualization but is not used to derive direction,
		//   which avoids "tangent flips" when sampling the closest point to the pawn.
	}
	// Fallback: invalid path state (e.g., no points); stop to avoid running forever.
	StopMovement();
}

void UClickToMoveComponent::FindPathToLocation()
{
	// Only short presses build an autorun path; long holds already moved the pawn.
	// This produces the common ARPG behavior: hold to steer; click to autorun.
	if (FollowTime > ShortPressThreshold)
	{
		FollowTime = 0.f;
		SetIsTargeting(false);
		return;
	}

	APawn* Pawn = GetControlledPawn();
	// If there is no pawn or spline, abort safely (cannot follow a path).
	if (!Pawn || !Spline)
	{
		FollowTime = 0.f;
		SetIsTargeting(false);
		return;
	}

	// Project desired destination to navmesh first (clicks on static mesh should still move).
	// This guarantees the path target is on a navigable surface.
	FVector GoalOnNav = CachedDestination;
	if (!ProjectPointToNavmesh(CachedDestination, GoalOnNav))
	{
		// No valid nav projection; abort cleanly without starting autorun.
		FollowTime = 0.f;
		SetIsTargeting(false);
		return;
	}
	
	// Build a nav path synchronously (fine for single-click flows).
	// For continuous updates (e.g., click-drag path preview), consider async path queries.
	if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		this, Pawn->GetActorLocation(), GoalOnNav, Pawn))
	{
		// Ensure the path is valid and contains at least one segment (start + goal).
		if (NavPath->IsValid() && !NavPath->PathPoints.IsEmpty())
		{

			// Cache points for accurate sequential following.
			// Index 0 is the starting point (typically the pawn's current location).
			PathPoints = NavPath->PathPoints;   // <-- PathPoints is cached here
			PathIndex  = 1;                     // 0 is start (pawn location); begin with the next point
			
			// Populate spline for optional visualization or debug math.
			Spline->ClearSplinePoints(false);
			for (const FVector& PathPoint : NavPath->PathPoints)
			{
				Spline->AddSplinePoint(PathPoint, ESplineCoordinateSpace::World, false);
				#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				// Visualize each path vertex to help diagnose navmesh cornering.
				DrawDebugSphere(GetWorld(), PathPoint, 12.f, 8, FColor::Green, false, 5.f);
				#endif
			}
			Spline->UpdateSpline();

			// Use final path point as authoritative final destination (useful for HUD/UX).
			CachedDestination = NavPath->PathPoints.Last();

			// Enable autorun and ticking to start following the path next frame.
			SetComponentTickEnabled(true);
			SetAutoRunActive(true);
		}
	}

	// Reset transient input state for the next click cycle, regardless of success or failure.
	FollowTime = 0.f;
	SetIsTargeting(false);
}