// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTree;

UCLASS()
class BRIDGEANDBLADE_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Optional: keep a BehaviorTree property if you want to run a BT per-pawn later.
    UPROPERTY(EditAnywhere, Category = "AI")
    UBehaviorTree* BehaviorTree;
};
