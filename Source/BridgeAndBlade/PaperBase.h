// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include "PaperCharacter.h"
#include "CoreMinimal.h"
#include "PaperFlipbookComponent.h"
#include "PaperBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPaperFlipbook;
class UPaperFlipbookComponent;

/**
 * 
 */
UCLASS()
class BRIDGEANDBLADE_API APaperBase : public APaperCharacter
{
	GENERATED_BODY()
	
public:
	// Sets default values for this pawn's properties
	APaperBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay();

public:
	// Called every frame
	virtual void Tick(float DeltaTime);

public:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CharacterMesh;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* WalkUpFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* WalkDownFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* WalkSideFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* AttackUpFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* AttackDownFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* AttackSideFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* IdleFlipbook;

	bool bHasMoved = false;

	UPROPERTY(EditAnywhere)
	int health = 10;

	UPROPERTY(EditAnywhere)
	int moveSpeed = 0;

	void UpdateAnimation();
	bool bIsMoving = false;

	UPROPERTY(EditAnywhere)
	TArray<FString> itemDrops;

	void die(TArray<FString> drops);
};
