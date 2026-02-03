// Fill out your copyright notice in the Description page of Project Settings.

#include "PaperCharPlayerController.h"
#include "Engine/EngineTypes.h"

APaperCharPlayerController::APaperCharPlayerController()
{
	// reasonable defaults (can be overridden per-instance in editor)
	bShowMouseCursorOnPossess = true;
	bEnableClickEventsOnPossess = true;
	bEnableMouseOverEventsOnPossess = true;
}

void APaperCharPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Apply editor-configured cursor settings at begin play
	bShowMouseCursor = bShowMouseCursorOnPossess;
	bEnableClickEvents = bEnableClickEventsOnPossess;
	bEnableMouseOverEvents = bEnableMouseOverEventsOnPossess;
}

void APaperCharPlayerController::SetShowCursor(bool bShow)
{
	bShowMouseCursor = bShow;
	// If enabling cursor at runtime also enable click/hover events for convenience.
	if (bShow)
	{
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
}

bool APaperCharPlayerController::GetHitUnderCursorByChannel(TEnumAsByte<ECollisionChannel> TraceChannel, FHitResult& OutHit) const
{
	// Convert ECollisionChannel to ETraceTypeQuery for the API
	const ETraceTypeQuery TraceQuery = UEngineTypes::ConvertToTraceType(TraceChannel);
	return const_cast<APaperCharPlayerController*>(this)->GetHitResultUnderCursorByChannel(TraceQuery, true, OutHit);
}

bool APaperCharPlayerController::GetMouseWorldPointAtPlane(float PlaneZ, FVector& OutWorldPoint) const
{
	FVector WorldOrigin;
	FVector WorldDirection;
	if (!const_cast<APaperCharPlayerController*>(this)->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return false;
	}

	// Intersect ray with horizontal plane at Z = PlaneZ
	if (!FMath::IsNearlyZero(WorldDirection.Z))
	{
		const float T = (PlaneZ - WorldOrigin.Z) / WorldDirection.Z;
		OutWorldPoint = WorldOrigin + WorldDirection * T;
	}
	else
	{
		// Ray parallel to plane — pick a far point along direction
		OutWorldPoint = WorldOrigin + WorldDirection * 10000.0f;
	}
	return true;
}

