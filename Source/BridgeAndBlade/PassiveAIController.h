// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.h"
#include "PassiveAIController.generated.h"

/**
 * 
 */
UCLASS()
class BRIDGEANDBLADE_API APassiveAIController : public AEnemyAIController
{
    GENERATED_BODY()

public:
    APassiveAIController();

    virtual void OnPossess(APawn* InPawn) override;

    // Override base chasing behavior so passive enemies never enter chasing state
    virtual void HandleChasing(float DeltaSeconds) override;
};
