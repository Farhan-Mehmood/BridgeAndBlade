// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PaperBase.h"
#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "PaperChar.generated.h"

class UInputMappingContext;
class UInputAction;
//class USceneComponent;
class UCameraComponent;
//class UFloatingPawnMovement;

UCLASS()
class BRIDGEANDBLADE_API APaperChar : public APaperBase
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

	//UPROPERTY(EditAnywhere, Category = "Components")
	//USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, Category = "Components")
	UCameraComponent* Camera;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//UFloatingPawnMovement* MovementComponent;

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
