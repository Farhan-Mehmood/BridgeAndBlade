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
	// Expose wind-up state so controllers can respect it
	bool IsWindingUp() const { return bIsWindingUp; }

	// Attempt an attack against TargetPawn. Returns true if an attack was executed (or wind-up started).
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

	// Wind-up time (seconds) before the attack is actually applied (plays wind-up animation during this time)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float WindupTime = 0.25f;

	// Optional: separate wind-up flipbooks (plays during the WindupTime). If null, the regular Attack*Flipbooks are used.
	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* WindupUpFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* WindupDownFlipbook;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UPaperFlipbook* WindupSideFlipbook;

private:
	// Time of the last performed attack (GetWorld()->GetTimeSeconds())
	float LastAttackTime = -FLT_MAX;

	// Whether we're currently winding up an attack
	bool bIsWindingUp = false;

	// Timer handle for windup
	FTimerHandle WindupTimerHandle;

	// Target pawn stored during wind-up
	TWeakObjectPtr<APawn> PendingTarget;

	// Called when wind-up finishes to apply the damage
	void ExecuteAttack();
};
