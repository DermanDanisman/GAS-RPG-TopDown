// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "Components/ClickToMoveComponent.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

// Constructor: allow ticking, but we enable it only during autorun.
UClickToMoveComponent::UClickToMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false); // client-driven; CharacterMovement replicates
}

void UClickToMoveComponent::BeginPlay()
{
	Super::BeginPlay();
	
	EnsureSplineNoAttach();

	// Start with ticking disabled; autorun flow will enable as needed.
	SetComponentTickEnabled(false);
}

void UClickToMoveComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Only drive autorun path following when active.
	if (bIsAutoRunning)
	{
		AutoRun();
	}
}

void UClickToMoveComponent::OnClickPressed()
{
	// Reset hold state and stop any ongoing autorun.
	SetAutoRunActive(false);
	FollowTime = 0.f;
	SetComponentTickEnabled(false);
}

void UClickToMoveComponent::OnClickHeld(const bool bUseInternalHitResult, const FHitResult& InHitResult)
{
	// Local-only guard: only the local PlayerController should drive movement.
	const APlayerController* PC = GetOwnerPC();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// If we're targeting, do not consume movement input.
	if (bIsTargeting)
	{
		return;
	}

	// Accumulate hold time (used on release to decide short-press autorun vs hold-to-move).
	if (UWorld* World = GetWorld())
	{
		FollowTime += World->GetDeltaSeconds();
	}

	// Either do an internal trace or use the provided hit result from elsewhere (e.g., HighlightInteraction).
	if (bUseInternalHitResult)
	{
		FHitResult Hit;
		if (PC->GetHitResultUnderCursor(CursorTraceChannel, /*bTraceComplex=*/false, Hit))
		{
			CachedDestination = Hit.ImpactPoint;
			ApplyMoveToward(CachedDestination);
		}
	}
	else if (InHitResult.bBlockingHit)
	{
		CachedDestination = InHitResult.ImpactPoint;
		ApplyMoveToward(CachedDestination);
	}
}

void UClickToMoveComponent::OnClickReleased()
{
	FindPathToLocation();
}

USplineComponent* UClickToMoveComponent::EnsureSplineNoAttach()
{
	if (!Spline)
	{
		// Owner as Outer ties lifetimes together; left unattached/unregistered by design.
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
	return nullptr;
}

APlayerController* UClickToMoveComponent::GetOwnerPC() const
{
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
	SetAutoRunActive(false);
	SetComponentTickEnabled(false);
}

void UClickToMoveComponent::ApplyMoveToward(const FVector& DestinationWorld) const
{
	// Local-only guard
	const APlayerController* PC = GetOwnerPC();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// Drive movement toward the given destination.
	if (APawn* Pawn = GetControlledPawn())
	{
		const FVector Direction = (DestinationWorld - Pawn->GetActorLocation()).GetSafeNormal();
		if (!Direction.IsNearlyZero())
		{
			Pawn->AddMovementInput(Direction, 1.f);
		}
	}
}

void UClickToMoveComponent::AutoRun()
{
	// Local-only guard
	const APlayerController* PC = GetOwnerPC();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	APawn* Pawn = GetControlledPawn();
	if (!Pawn || !Spline) return;

	// Move along the spline by taking direction at the closest point to the pawn.
	const FVector PawnLoc = Pawn->GetActorLocation();
	const FVector ClosestOnSpline = Spline->FindLocationClosestToWorldLocation(PawnLoc, ESplineCoordinateSpace::World);
	const FVector Direction = Spline->FindDirectionClosestToWorldLocation(ClosestOnSpline, ESplineCoordinateSpace::World);

	if (!Direction.IsNearlyZero())
	{
		Pawn->AddMovementInput(Direction, 1.f);
	}

	// Stop when the pawn is within AcceptanceRadius of the final destination.
	if (FVector::DistSquared(PawnLoc, CachedDestination) <= FMath::Square(AcceptanceRadius))
	{
		StopMovement();
	}
}

void UClickToMoveComponent::FindPathToLocation()
{
	// Only short presses build an autorun path; long holds already moved the pawn.
	if (FollowTime > ShortPressThreshold)
	{
		FollowTime = 0.f;
		SetIsTargeting(false);
		return;
	}

	APawn* Pawn = GetControlledPawn();
	if (!Pawn || !Spline)
	{
		FollowTime = 0.f;
		SetIsTargeting(false);
		return;
	}
	
	// Build a nav path synchronously (fine for single-click flows).
	if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		this, Pawn->GetActorLocation(), CachedDestination, Pawn))
	{
		if (NavPath->IsValid() && !NavPath->PathPoints.IsEmpty())
		{
			Spline->ClearSplinePoints(false);
			for (const FVector& P : NavPath->PathPoints)
			{
				Spline->AddSplinePoint(P, ESplineCoordinateSpace::World, false);
				#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				DrawDebugSphere(GetWorld(), P, 12.f, 8, FColor::Green, false, 5.f);
				#endif
			}
			Spline->UpdateSpline();

			// Use final path point as authoritative final destination.
			CachedDestination = NavPath->PathPoints.Last();

			// Enable autorun and ticking.
			SetComponentTickEnabled(true);
			SetAutoRunActive(true);
		}
	}

	// Reset transient input state for the next click cycle.
	FollowTime = 0.f;
	SetIsTargeting(false);
}