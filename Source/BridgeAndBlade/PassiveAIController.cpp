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

void APassiveAIController::HandleChasing(float DeltaSeconds)
{
    SetState(EEnemyState::Patrolling);
}