// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTree;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Patrolling UMETA(DisplayName = "Patrolling"),
    Chasing UMETA(DisplayName = "Chasing"),
    Attacking UMETA(DisplayName = "Attacking"),
    Returning UMETA(DisplayName = "Returning")
};

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

    // Patrol settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolWaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float SightRange = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    bool bRequireLineOfSight = true;

private:
    // Spawn location (center of patrol zone)
    FVector SpawnLocation;

    // Current AI state
    EEnemyState CurrentState;

    // Patrol point timer
    float PatrolTimer;

    // Current patrol target location
    FVector CurrentPatrolPoint;

    // Functions
    void HandlePatrolling(float DeltaSeconds);
    void HandleChasing(float DeltaSeconds);
    void HandleAttacking(float DeltaSeconds);
    void HandleReturning(float DeltaSeconds);

    FVector GetRandomPatrolPoint() const;
    bool IsPlayerInPatrolZone(const APawn* PlayerPawn) const;
    bool CanSeePlayer(const APawn* PlayerPawn) const;
    void SetState(EEnemyState NewState);
};
