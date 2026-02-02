// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "PaperBase.h"
#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h" // for EPathFollowingStatus

AEnemyAIController::AEnemyAIController()
{
    // allow Tick on this controller
    PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Keep behavior tree startup here if you add BT assets on the pawn later.
    // if (APaperBase* EnemyPawn = Cast<APaperBase>(InPawn))
    // {
    //     if (EnemyPawn->BehaviorTree)
    //     {
    //         RunBehaviorTree(EnemyPawn->BehaviorTree);
    //     }
    // }
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyAIController has no controlled pawn"));
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyAIController cannot find player pawn"));
        return;
    }

    APaperEnemy* EnemyPawn = Cast<APaperEnemy>(ControlledPawn);

    // Use the enemy's AttackRange as the reference. Distance check uses the full AttackRange.
    const float EffectiveAttackRange = (EnemyPawn != nullptr) ? EnemyPawn->AttackRange : 120.0f;
    const float AttackRangeSqr = EffectiveAttackRange * EffectiveAttackRange;

    const FVector MyLocation = ControlledPawn->GetActorLocation();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float DistanceSqr = FVector::DistSquared(MyLocation, PlayerLocation);

    // If already inside the real attack range, stop and attack immediately.
    if (DistanceSqr <= AttackRangeSqr)
    {
        StopMovement();

        if (EnemyPawn)
        {
            UE_LOG(LogTemp, Log, TEXT("Enemy %s in attack range, attacking player"), *ControlledPawn->GetName());
            EnemyPawn->Attack(PlayerPawn); // Attack implements cooldown and uses DamageAmount
        }

        return;
    }

    // Not yet in attack range -> request movement with a tight acceptance so the AI moves
    // inside the AttackRange before stopping. Use a small acceptance to avoid early stopping.
    const float TightAcceptance = 20.0f; // small value so pathing moves very close to the player

    // Avoid re-requesting MoveTo every tick if already moving (reduces churn).
    if (GetMoveStatus() != EPathFollowingStatus::Moving)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Enemy %s moving toward player (tight acceptance = %f, attack = %f)"), *ControlledPawn->GetName(), TightAcceptance, EffectiveAttackRange);
        MoveToActor(PlayerPawn, TightAcceptance);
    }
}

