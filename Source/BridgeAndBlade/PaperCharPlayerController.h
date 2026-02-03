// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PaperCharPlayerController.generated.h"

/**
 * PlayerController that can show the mouse cursor and enable simple click/hover handling.
 */
UCLASS()
class BRIDGEANDBLADE_API APaperCharPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APaperCharPlayerController();

protected:
	virtual void BeginPlay() override;

public:
	// Editor-configurable defaults for cursor behavior when this controller begins play
	UPROPERTY(EditAnywhere, Category = "Cursor")
	bool bShowMouseCursorOnPossess = true;

	UPROPERTY(EditAnywhere, Category = "Cursor")
	bool bEnableClickEventsOnPossess = true;

	UPROPERTY(EditAnywhere, Category = "Cursor")
	bool bEnableMouseOverEventsOnPossess = true;

	// Toggle cursor at runtime (callable from Blueprints)
	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void SetShowCursor(bool bShow);

	// Return an accurate world location hit under the cursor by the given trace channel (returns false if no hit)
	UFUNCTION(BlueprintCallable, Category = "Cursor")
	bool GetHitUnderCursorByChannel(TEnumAsByte<ECollisionChannel> TraceChannel, FHitResult& OutHit) const;

	// Deproject mouse to world and intersect with a plane at Z=PlaneZ; returns false if deprojection fails.
	UFUNCTION(BlueprintCallable, Category = "Cursor")
	bool GetMouseWorldPointAtPlane(float PlaneZ, FVector& OutWorldPoint) const;
};
