// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperBase.h"
#include "PaperEnemy.generated.h"

class APawn;

UCLASS()
class BRIDGEANDBLADE_API APaperEnemy : public APaperBase
{
	GENERATED_BODY()

public:
	APaperEnemy();

	virtual void BeginPlay() override;

	// Attempt an attack against TargetPawn. Returns true if an attack was executed.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool Attack(APawn* TargetPawn);

	// Check whether the enemy can attack (cooldown)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanAttack() const;

	// Range (units) for melee attack
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 120.0f;

	// Seconds between attacks
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 1.25f;

	// Damage applied when attack hits
	UPROPERTY(EditAnywhere, Category = "Combat")
	float DamageAmount = 10.0f;

private:
	// Time of the last performed attack (GetWorld()->GetTimeSeconds())
	float LastAttackTime = -FLT_MAX;
};
