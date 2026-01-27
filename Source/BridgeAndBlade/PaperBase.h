// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include <list>
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

	UPROPERTY(EditAnywhere, Category = "Components")
	USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CharacterMesh;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* WalkUpFlipbook;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* WalkDownFlipbook;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* WalkSideFlipbook;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* AttackUpFlipbook;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* AttackDownFlipbook;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* AttackSideFlipbook;

	UPROPERTY(EditAnywhere)
	UPaperFlipbook* IdleFlipbook;

	UPROPERTY(EditAnywhere)
	int health = 10;

	UPROPERTY(EditAnywhere)
	int moveSpeed = 0;

	void UpdateAnimation();
	bool bIsMoving = false;

	std::list<std::string> drops;
};
