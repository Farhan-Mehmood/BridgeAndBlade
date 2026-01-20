// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "PaperChar.generated.h"

class UInputMappingContext;
class UInputAction;
class USceneComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UPaperFlipbookComponent;

UCLASS()
class BRIDGEANDBLADE_API APaperChar : public APaperCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APaperChar();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UPaperFlipbook* IdleFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UPaperFlipbook* RunUpFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UPaperFlipbook* RunDownFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UPaperFlipbook* RunSideFlipbook;

	bool bIsRunning = false;

	void UpdateAnimation();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PlayerMesh;


	UPROPERTY(EditAnywhere, Category = "Components")
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UFloatingPawnMovement* MovementComponent;

	// Input
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveUpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveRightAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ZoomCameraAction;

	// Movement input handlers
	void MoveUp(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void ZoomCamera(const FInputActionValue& Value);
};
