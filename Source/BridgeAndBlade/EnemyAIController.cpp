// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "PaperBase.h"
#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

AEnemyAIController::AEnemyAIController()
{
    // allow Tick on this controller
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentState = EEnemyState::Patrolling;
    PatrolTimer = 0.0f;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Store spawn location as the center of the patrol zone
    if (InPawn)
    {
        SpawnLocation = InPawn->GetActorLocation();
        
        // Project spawn location to NavMesh to get the correct Z-height
        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLoc;
            if (NavSys->ProjectPointToNavigation(SpawnLocation, NavLoc, FVector(500, 500, 500)))
            {
                SpawnLocation = NavLoc.Location;
                UE_LOG(LogTemp, Warning, TEXT("Enemy %s spawn location projected to NavMesh: %s"), 
                    *InPawn->GetName(), *SpawnLocation.ToString());
            }
        }
        
        CurrentPatrolPoint = GetRandomPatrolPoint();
        SetState(EEnemyState::Patrolling);
        
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s spawned at %s, patrol radius: %f"), 
            *InPawn->GetName(), *SpawnLocation.ToString(), PatrolRadius);
        
        // Start moving to first patrol point immediately
        MoveToLocation(CurrentPatrolPoint, 50.0f);
    }
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    // If the enemy is winding up an attack, don't do anything else
    APaperEnemy* EnemyPawn = Cast<APaperEnemy>(ControlledPawn);
    if (EnemyPawn && EnemyPawn->IsWindingUp())
    {
        StopMovement();
        return;
    }

    // Debug: Draw patrol radius
    if (GetWorld())
    {
        DrawDebugCircle(GetWorld(), SpawnLocation, PatrolRadius, 32, FColor::Green, false, -1.0f, 0, 2.0f, FVector(0, 0, 1), FVector(0, 1, 0));
        DrawDebugSphere(GetWorld(), CurrentPatrolPoint, 30.0f, 8, FColor::Yellow, false, -1.0f, 0, 2.0f);
    }

    // Handle state machine
    switch (CurrentState)
    {
        case EEnemyState::Patrolling:
            HandlePatrolling(DeltaSeconds);
            break;
        case EEnemyState::Chasing:
            HandleChasing(DeltaSeconds);
            break;
        case EEnemyState::Attacking:
            HandleAttacking(DeltaSeconds);
            break;
        case EEnemyState::Returning:
            HandleReturning(DeltaSeconds);
            break;
    }
}

void AEnemyAIController::HandlePatrolling(float DeltaSeconds)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Check if player is in patrol zone and visible EVERY tick
    if (PlayerPawn && IsPlayerInPatrolZone(PlayerPawn) && CanSeePlayer(PlayerPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s spotted player! Switching to Chase"), *ControlledPawn->GetName());
        StopMovement(); // Stop current patrol movement immediately
        SetState(EEnemyState::Chasing);
        return;
    }

    // Continue patrolling
    const FVector CurrentLocation = ControlledPawn->GetActorLocation();
    const float DistanceToPatrolPoint = FVector::Dist2D(CurrentLocation, CurrentPatrolPoint);

    if (DistanceToPatrolPoint < 50.0f) // Reached patrol point
    {
        // Wait at patrol point
        PatrolTimer += DeltaSeconds;
        
        if (PatrolTimer >= PatrolWaitTime)
        {
            // Pick new random patrol point
            CurrentPatrolPoint = GetRandomPatrolPoint();
            PatrolTimer = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("Enemy %s picked new patrol point at %s"), 
                *ControlledPawn->GetName(), *CurrentPatrolPoint.ToString());
            
            MoveToLocation(CurrentPatrolPoint, 50.0f);
        }
        else
        {
            StopMovement();
        }
    }
    else
    {
        // Move to patrol point if not already moving
        EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
        if (MoveStatus != EPathFollowingStatus::Moving)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy %s attempting to move to patrol point. Distance: %f, MoveStatus: %d"), 
                *ControlledPawn->GetName(), DistanceToPatrolPoint, (int32)MoveStatus);
            
            FAIMoveRequest MoveRequest(CurrentPatrolPoint);
            MoveRequest.SetAcceptanceRadius(50.0f);
            FNavPathSharedPtr NavPath;
            FPathFollowingRequestResult Result = MoveTo(MoveRequest, &NavPath);
            
            // Check if path was found
            if (!NavPath.IsValid() || NavPath->IsPartial())
            {
                UE_LOG(LogTemp, Error, TEXT("Enemy %s: No valid path to patrol point!"), *ControlledPawn->GetName());
                UE_LOG(LogTemp, Error, TEXT("  Current Location: %s"), *CurrentLocation.ToString());
                UE_LOG(LogTemp, Error, TEXT("  Target Location: %s"), *CurrentPatrolPoint.ToString());
                
                // Pick a new patrol point immediately
                UE_LOG(LogTemp, Warning, TEXT("  Generating new patrol point..."));
                CurrentPatrolPoint = GetRandomPatrolPoint();
                PatrolTimer = PatrolWaitTime; // Force immediate retry on next tick
            }
        }
    }
}

void AEnemyAIController::HandleChasing(float DeltaSeconds)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player lost, returning"), *ControlledPawn->GetName());
        StopMovement();
        SetState(EEnemyState::Returning);
        return;
    }

    // Check if player left the patrol zone
    if (!IsPlayerInPatrolZone(PlayerPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player left patrol zone, returning"), *ControlledPawn->GetName());
        StopMovement();
        SetState(EEnemyState::Returning);
        return;
    }

    // If lost sight of player, return to patrol
    if (bRequireLineOfSight && !CanSeePlayer(PlayerPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Lost sight of player, returning"), *ControlledPawn->GetName());
        StopMovement();
        SetState(EEnemyState::Returning);
        return;
    }

    APaperEnemy* EnemyPawn = Cast<APaperEnemy>(ControlledPawn);
    const float EffectiveAttackRange = (EnemyPawn != nullptr) ? EnemyPawn->AttackRange : 120.0f;
    const float AttackRangeSqr = EffectiveAttackRange * EffectiveAttackRange;

    const FVector MyLocation = ControlledPawn->GetActorLocation();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float DistanceSqr = FVector::DistSquared(MyLocation, PlayerLocation);

    // If in attack range, switch to attacking state
    if (DistanceSqr <= AttackRangeSqr)
    {
        SetState(EEnemyState::Attacking);
        return;
    }

    // Chase the player - issue move command every tick to track moving target
    const float TightAcceptance = 20.0f;
    MoveToActor(PlayerPawn, TightAcceptance);
}

void AEnemyAIController::HandleAttacking(float DeltaSeconds)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        StopMovement();
        SetState(EEnemyState::Returning);
        return;
    }

    // If player left the patrol zone, return to patrol
    if (!IsPlayerInPatrolZone(PlayerPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player left patrol zone during attack, returning"), *ControlledPawn->GetName());
        StopMovement();
        SetState(EEnemyState::Returning);
        return;
    }

    APaperEnemy* EnemyPawn = Cast<APaperEnemy>(ControlledPawn);
    const float EffectiveAttackRange = (EnemyPawn != nullptr) ? EnemyPawn->AttackRange : 120.0f;
    const float AttackRangeSqr = EffectiveAttackRange * EffectiveAttackRange;

    const FVector MyLocation = ControlledPawn->GetActorLocation();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float DistanceSqr = FVector::DistSquared(MyLocation, PlayerLocation);

    // If player moved out of attack range, chase again
    if (DistanceSqr > AttackRangeSqr)
    {
        SetState(EEnemyState::Chasing);
        return;
    }

    // Stop and attack
    StopMovement();

    if (EnemyPawn)
    {
        EnemyPawn->Attack(PlayerPawn);
    }
}

void AEnemyAIController::HandleReturning(float DeltaSeconds)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Check if player re-entered patrol zone while returning
    if (PlayerPawn && IsPlayerInPatrolZone(PlayerPawn) && CanSeePlayer(PlayerPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player re-entered patrol zone, chasing again"), *ControlledPawn->GetName());
        StopMovement();
        SetState(EEnemyState::Chasing);
        return;
    }

    // Move back to spawn location
    const FVector CurrentLocation = ControlledPawn->GetActorLocation();
    const float DistanceToSpawn = FVector::Dist2D(CurrentLocation, SpawnLocation);

    UE_LOG(LogTemp, Verbose, TEXT("Enemy %s returning to spawn. Distance: %f"), *ControlledPawn->GetName(), DistanceToSpawn);

    if (DistanceToSpawn < 100.0f) // Reached spawn area
    {
        // Resume patrolling with a new random point
        CurrentPatrolPoint = GetRandomPatrolPoint();
        PatrolTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Enemy %s: Reached spawn, resuming patrol"), *ControlledPawn->GetName());
        SetState(EEnemyState::Patrolling);
        MoveToLocation(CurrentPatrolPoint, 50.0f);
    }
    else
    {
        // Always move to spawn location every tick while returning
        // Don't check if already moving - just keep updating the move command
        UE_LOG(LogTemp, Verbose, TEXT("Enemy %s moving to spawn at %s"), *ControlledPawn->GetName(), *SpawnLocation.ToString());
        MoveToLocation(SpawnLocation, 100.0f);
    }
}

FVector AEnemyAIController::GetRandomPatrolPoint() const
{
    // Generate a random point within the patrol radius around spawn location
    const float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
    const float RandomDistance = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    // Use the SAME Z-height as spawn location (which was already projected to NavMesh)
    FVector PatrolPoint = FVector(
        SpawnLocation.X + FMath::Cos(RandomAngle) * RandomDistance,
        SpawnLocation.Y + FMath::Sin(RandomAngle) * RandomDistance,
        SpawnLocation.Z  // Keep exact same Z as spawn
    );
    
    // Project the point onto the NavMesh, but preserve the Z if projection fails
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation ProjectedLocation;
        // Use tight vertical search extent to stay on same plane
        FVector SearchExtent(200.0f, 200.0f, 10.0f);
        
        if (NavSys->ProjectPointToNavigation(PatrolPoint, ProjectedLocation, SearchExtent))
        {
            // Force the projected point to use spawn's Z coordinate
            ProjectedLocation.Location.Z = SpawnLocation.Z;
            PatrolPoint = ProjectedLocation.Location;
            
            UE_LOG(LogTemp, Verbose, TEXT("Generated valid patrol point at %s (angle: %.2f, dist: %.2f)"), 
                *PatrolPoint.ToString(), RandomAngle, RandomDistance);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not project patrol point, using spawn Z anyway"));
            // Keep the PatrolPoint we already calculated with spawn's Z
        }
    }

    return PatrolPoint;
}

bool AEnemyAIController::IsPlayerInPatrolZone(const APawn* PlayerPawn) const
{
    if (!PlayerPawn) return false;

    const float DistanceToSpawn = FVector::Dist2D(PlayerPawn->GetActorLocation(), SpawnLocation);
    const bool bInZone = DistanceToSpawn <= PatrolRadius;
    
    // Debug visualization
    if (GetWorld())
    {
        FColor DebugColor = bInZone ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), SpawnLocation, PlayerPawn->GetActorLocation(), DebugColor, false, 0.1f, 0, 2.0f);
    }
    
    return bInZone;
}

bool AEnemyAIController::CanSeePlayer(const APawn* PlayerPawn) const
{
    if (!PlayerPawn || !GetPawn()) return false;

    const FVector MyLocation = GetPawn()->GetActorLocation();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float DistanceSqr = FVector::DistSquared(MyLocation, PlayerLocation);
    const float SightRangeSqr = SightRange * SightRange;

    // Check if player is within sight range
    if (DistanceSqr > SightRangeSqr)
    {
        return false;
    }

    // Optionally perform line-of-sight check
    if (bRequireLineOfSight && GetWorld())
    {
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetPawn());
        QueryParams.AddIgnoredActor(PlayerPawn);

        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            MyLocation,
            PlayerLocation,
            ECC_Visibility,
            QueryParams
        );

        // Debug visualization
        if (bHit)
        {
            DrawDebugLine(GetWorld(), MyLocation, PlayerLocation, FColor::Red, false, 0.1f, 0, 2.0f);
            DrawDebugPoint(GetWorld(), HitResult.Location, 10.0f, FColor::Orange, false, 0.1f);
        }
        else
        {
            DrawDebugLine(GetWorld(), MyLocation, PlayerLocation, FColor::Cyan, false, 0.1f, 0, 2.0f);
        }

        // If trace hit something, we don't have line of sight
        return !bHit;
    }

    return true;
}

void AEnemyAIController::SetState(EEnemyState NewState)
{
    if (CurrentState != NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s changing state: %d -> %d"), 
            *GetPawn()->GetName(), (int32)CurrentState, (int32)NewState);
        
        CurrentState = NewState;

        // Reset relevant timers/variables on state change
        if (NewState == EEnemyState::Patrolling)
        {
            PatrolTimer = 0.0f;
        }
    }
}

