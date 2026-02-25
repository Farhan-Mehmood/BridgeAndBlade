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
#include "GameFramework/Character.h"

AEnemyAIController::AEnemyAIController()
{
    // allow Tick on this controller
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentState = EEnemyState::Patrolling;
    PatrolTimer = 0.0f;
    LastKnownPlayerLocation = FVector::ZeroVector;
    ChaseTimeOutsideZone = 0.0f;
    MaxChaseTimeOutsideZone = FMath::FRandRange(5.0f, 10.0f);

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
        
        // Set up collision detection to avoid getting stuck
        if (ACharacter* EnemyCharacter = Cast<ACharacter>(InPawn))
        {
            EnemyCharacter->OnActorHit.AddDynamic(this, &AEnemyAIController::OnEnemyHit);
        }
        
        // Start moving to first patrol point immediately
        MoveToLocation(CurrentPatrolPoint, 50.0f);
    }
}

void AEnemyAIController::OnEnemyHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    // Don't react to player collisions - those are handled in the chase/attack logic
    if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        return;
    }

    // Only react during patrol - during chase/attack we want normal behavior
    if (CurrentState != EEnemyState::Patrolling)
    {
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Enemy %s hit %s, picking new patrol direction"), 
        *ControlledPawn->GetName(), *OtherActor->GetName());

    // Get current facing direction
    FVector CurrentForward = ControlledPawn->GetActorForwardVector();
    CurrentForward.Z = 0.0f;
    CurrentForward.Normalize();

    // Pick a new direction that's at least 90 degrees away from current direction
    float MinAngleDeviation = 90.0f;
    float MaxAngleDeviation = 270.0f;
    float RandomAngleOffset = FMath::FRandRange(MinAngleDeviation, MaxAngleDeviation);
    
    // Randomly choose left or right turn
    if (FMath::RandBool())
    {
        RandomAngleOffset = -RandomAngleOffset;
    }

    // Calculate new direction
    FRotator CurrentRotation = CurrentForward.Rotation();
    FRotator NewRotation = CurrentRotation + FRotator(0, RandomAngleOffset, 0);
    FVector NewDirection = NewRotation.Vector();
    NewDirection.Z = 0.0f;
    NewDirection.Normalize();

    // Calculate new patrol point in that direction
    float DistanceFromSpawn = FVector::Dist2D(ControlledPawn->GetActorLocation(), SpawnLocation);
    float RemainingRadius = PatrolRadius - DistanceFromSpawn;
    
    // If we're near the edge of patrol radius, aim back toward spawn
    if (RemainingRadius < PatrolRadius * 0.3f)
    {
        FVector ToSpawn = SpawnLocation - ControlledPawn->GetActorLocation();
        ToSpawn.Z = 0.0f;
        ToSpawn.Normalize();
        NewDirection = ToSpawn;
    }

    float NewPatrolDistance = FMath::FRandRange(PatrolRadius * 0.4f, PatrolRadius * 0.8f);
    FVector NewPatrolPoint = ControlledPawn->GetActorLocation() + (NewDirection * NewPatrolDistance);
    NewPatrolPoint.Z = SpawnLocation.Z;

    // Project to navmesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(NewPatrolPoint, NavLocation, FVector(500, 500, 100)))
        {
            NewPatrolPoint = NavLocation.Location;
            NewPatrolPoint.Z = SpawnLocation.Z;
        }
    }

    // Update patrol point and immediately move there
    CurrentPatrolPoint = NewPatrolPoint;
    PatrolTimer = 0.0f;
    StopMovement();
    MoveToLocation(CurrentPatrolPoint, 50.0f);

    UE_LOG(LogTemp, Log, TEXT("Enemy %s new patrol point: %s"), 
        *ControlledPawn->GetName(), *CurrentPatrolPoint.ToString());
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
        StopMovement();
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
                PatrolTimer = PatrolWaitTime;
            }
        }
    }

    // Check for obstacles ahead and turn if needed
    if (CheckForObstaclesAhead())
    {
        UE_LOG(LogTemp, Log, TEXT("Enemy %s detected obstacle ahead, changing direction"), *ControlledPawn->GetName());
        
        // Pick a new direction away from obstacle
        FVector CurrentForward = ControlledPawn->GetActorForwardVector();
        float TurnAngle = FMath::FRandRange(90.0f, 135.0f) * (FMath::RandBool() ? 1.0f : -1.0f);
        FRotator NewRotation = CurrentForward.Rotation() + FRotator(0, TurnAngle, 0);
        FVector NewDirection = NewRotation.Vector();
        
        float NewDistance = FMath::FRandRange(PatrolRadius * 0.4f, PatrolRadius * 0.7f);
        CurrentPatrolPoint = ControlledPawn->GetActorLocation() + (NewDirection * NewDistance);
        CurrentPatrolPoint.Z = SpawnLocation.Z;
        
        PatrolTimer = 0.0f;
        MoveToLocation(CurrentPatrolPoint, 50.0f);
        return;
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
        SetState(EEnemyState::Patrolling);
        ChaseTimeOutsideZone = 0.0f;
        return;
    }

    bool bPlayerInZone = IsPlayerInPatrolZone(PlayerPawn);
    bool bCanSeePlayer = CanSeePlayer(PlayerPawn);

    // Update last known location if we can see the player
    if (bCanSeePlayer)
    {
        LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        ChaseTimeOutsideZone = 0.0f; // Reset timer when we can see player
    }

    // If player left the patrol zone, start counting timeout
    if (!bPlayerInZone)
    {
        ChaseTimeOutsideZone += DeltaSeconds;
        
        if (ChaseTimeOutsideZone >= MaxChaseTimeOutsideZone)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player left patrol zone too long, returning"), *ControlledPawn->GetName());
            StopMovement();
            SetState(EEnemyState::Patrolling);
            ChaseTimeOutsideZone = 0.0f;
            return;
        }
    }

    // If lost sight of player, go to last known location
    if (bRequireLineOfSight && !bCanSeePlayer)
    {
        TimeSearchingLastKnown += DeltaSeconds;
        
        // Give up if we've been searching too long
        if (TimeSearchingLastKnown >= MaxSearchTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Searched too long, giving up"), *ControlledPawn->GetName());
            StopMovement();
            SetState(EEnemyState::Patrolling);
            TimeSearchingLastKnown = 0.0f;
            ChaseTimeOutsideZone = 0.0f;
            return;
        }
        
        const FVector MyLocation = ControlledPawn->GetActorLocation();
        const float DistanceToLastKnown = FVector::Dist2D(MyLocation, LastKnownPlayerLocation);
        
        if (DistanceToLastKnown > 50.0f)
        {
            // Only try to move if we have a valid path
            if (GetMoveStatus() != EPathFollowingStatus::Moving)
            {
                FAIMoveRequest MoveRequest(LastKnownPlayerLocation);
                MoveRequest.SetAcceptanceRadius(50.0f);
                FNavPathSharedPtr NavPath;
                MoveTo(MoveRequest, &NavPath);
                
                if (!NavPath.IsValid() || NavPath->IsPartial())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Can't reach last known location, returning to patrol"), *ControlledPawn->GetName());
                    StopMovement();
                    SetState(EEnemyState::Patrolling);
                    TimeSearchingLastKnown = 0.0f;
                    ChaseTimeOutsideZone = 0.0f;
                    return;
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Enemy %s: Lost sight, moving to last known location"), *ControlledPawn->GetName());
            return;
        }
        else
        {
            // Reached last known location but still can't see player
            UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Reached last known location, player not found, returning"), *ControlledPawn->GetName());
            StopMovement();
            SetState(EEnemyState::Patrolling);
            TimeSearchingLastKnown = 0.0f;
            ChaseTimeOutsideZone = 0.0f;
            return;
        }
    }
    else
    {
        // Can see player, reset search timer
        TimeSearchingLastKnown = 0.0f;
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
        ChaseTimeOutsideZone = 0.0f;
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
        SetState(EEnemyState::Patrolling);
        return;
    }

    // If player left the patrol zone, return to patrol
    if (!IsPlayerInPatrolZone(PlayerPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player left patrol zone during attack, returning"), *ControlledPawn->GetName());
        StopMovement();
        SetState(EEnemyState::Patrolling);
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
        SpawnLocation.Z
    );
    
    // Project the point onto the NavMesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation ProjectedLocation;
        FVector SearchExtent(200.0f, 200.0f, 10.0f);
        
        if (NavSys->ProjectPointToNavigation(PatrolPoint, ProjectedLocation, SearchExtent))
        {
            ProjectedLocation.Location.Z = SpawnLocation.Z;
            PatrolPoint = ProjectedLocation.Location;
            
            UE_LOG(LogTemp, Verbose, TEXT("Generated valid patrol point at %s (angle: %.2f, dist: %.2f)"), 
                *PatrolPoint.ToString(), RandomAngle, RandomDistance);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not project patrol point, using spawn Z anyway"));
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

bool AEnemyAIController::CheckForObstaclesAhead(float LookAheadDistance) const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn || !GetWorld()) return false;

    FVector StartLocation = ControlledPawn->GetActorLocation();
    FVector ForwardVector = ControlledPawn->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * LookAheadDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ControlledPawn);
    
    // Ignore the player - we want to chase them
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        QueryParams.AddIgnoredActor(PlayerPawn);
    }

    bool bHitSomething = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Pawn,
        QueryParams
    );

    return bHitSomething;
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
            ChaseTimeOutsideZone = 0.0f;
            TimeSearchingLastKnown = 0.0f;
        }
        else if (NewState == EEnemyState::Chasing)
        {
            MaxChaseTimeOutsideZone = FMath::FRandRange(5.0f, 10.0f);
            TimeSearchingLastKnown = 0.0f;
        }
    }
}

