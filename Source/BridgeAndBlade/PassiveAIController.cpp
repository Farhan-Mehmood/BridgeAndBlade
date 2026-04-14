// Fill out your copyright notice in the Description page of Project Settings.

#include "PassiveAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "PaperBase.h"
#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"

APassiveAIController::APassiveAIController()
{
    UE_LOG(LogTemp, Log, TEXT("APassiveAIController constructed (%s)"), *GetName());
}

void APassiveAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Log, TEXT("APassiveAIController::OnPossess called on controller=%s pawn=%s"),
        *GetName(),
        InPawn ? *InPawn->GetName() : TEXT("null"));

    // Ensure starting state is patrolling
    SetState(EEnemyState::Patrolling);
}

void APassiveAIController::HandleChasing(float DeltaSeconds)
{
    // Prevent chasing — force back to patrol state
    SetState(EEnemyState::Patrolling);

    if (GetPawn())
    {
        UE_LOG(LogTemp, Log, TEXT("Passive HandleChasing called on %s"), *GetPawn()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Passive HandleChasing called but GetPawn() == nullptr"));
    }
}