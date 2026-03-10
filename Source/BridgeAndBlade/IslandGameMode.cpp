// Fill out your copyright notice in the Description page of Project Settings.

#include "IslandGameMode.h"
#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

AIslandGameMode::AIslandGameMode()
{
    // Don't set DefaultPawnClass here - let it be configured in Blueprint
}

void AIslandGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Spawn environment objects once
    SpawnEnvironmentObjects();

    // Start the repeating spawn timer
    if (SpawnIntervalSeconds > 0.0f && EnemyClasses.Num() > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AIslandGameMode::TrySpawnTick, SpawnIntervalSeconds, true, 0.5f);
    }
}

void AIslandGameMode::TrySpawnTick()
{
    if (EnemyClasses.Num() == 0 || !GetWorld())
    {
        return;
    }

    // Clean up any far or null enemies first
    CleanupFarEnemies();

    // Limit concurrent enemies
    if (SpawnedEnemies.Num() >= MaxConcurrentEnemies)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    FVector SpawnLocation = GetRandomPointAroundPlayer(SpawnMinRadius, SpawnMaxRadius);

    // Project to navmesh so enemies can navigate
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (!NavSys->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500, 500, 500)))
        {
            // Skip spawn if no navmesh close to chosen point
            return;
        }
        SpawnLocation = NavLocation.Location;
    }

    // Pick random enemy class
    int32 Idx = FMath::RandRange(0, EnemyClasses.Num() - 1);
    TSubclassOf<APaperEnemy> EnemyClass = EnemyClasses.IsValidIndex(Idx) ? EnemyClasses[Idx] : nullptr;
    if (!EnemyClass) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APaperEnemy* SpawnedEnemy = GetWorld()->SpawnActor<APaperEnemy>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (SpawnedEnemy)
    {
        SpawnedEnemies.Add(SpawnedEnemy);
        UE_LOG(LogTemp, Log, TEXT("IslandGameMode: Spawned enemy %s at %s (active=%d)"), *SpawnedEnemy->GetName(), *SpawnLocation.ToString(), SpawnedEnemies.Num());
    }
}

void AIslandGameMode::CleanupFarEnemies()
{
    if (!GetWorld()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Iterate backwards so we can remove safely
    for (int32 i = SpawnedEnemies.Num() - 1; i >= 0; --i)
    {
        APaperEnemy* E = SpawnedEnemies[i];
        // Use IsValid to check for null / pending kill
        if (!IsValid(E))
        {
            SpawnedEnemies.RemoveAtSwap(i);
            continue;
        }

        float DistSq = FVector::DistSquared(E->GetActorLocation(), PlayerLocation);
        if (DistSq > (DespawnRadius * DespawnRadius))
        {
            UE_LOG(LogTemp, Log, TEXT("IslandGameMode: Despawning enemy %s (dist=%f)"), *E->GetName(), FMath::Sqrt(DistSq));
            E->Destroy();
            SpawnedEnemies.RemoveAtSwap(i);
        }
    }
}

FVector AIslandGameMode::GetRandomPointAroundPlayer(float MinRadius, float MaxRadius) const
{
    APawn* PlayerPawn = nullptr;
    if (GetWorld())
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
    if (!PlayerPawn)
    {
        // fallback to world origin
        return FVector::ZeroVector;
    }

    const FVector Center = PlayerPawn->GetActorLocation();
    const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
    const float Radius = FMath::FRandRange(MinRadius, MaxRadius);
    const float X = Center.X + FMath::Cos(Angle) * Radius;
    const float Y = Center.Y + FMath::Sin(Angle) * Radius;
    const float Z = Center.Z;

    return FVector(X, Y, Z);
}

void AIslandGameMode::SpawnEnvironmentObjects()
{
    if (EnvironmentActorClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("IslandGameMode: No environment objects configured for spawning"));
        return;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    int32 TotalSpawned = 0;

    for (int32 i = 0; i < EnvironmentObjectsToSpawn; ++i)
    {
        // Randomly pick what to spawn
        TSubclassOf<AActor> EnvClass = EnvironmentActorClasses[FMath::RandRange(0, EnvironmentActorClasses.Num() - 1)];

        // Get a random spot within the environment bounds
        FVector SpawnLocation = GetRandomLocationInBounds(IslandBoundsMin, IslandBoundsMax);

        // Try to snap to navmesh, though not critical for static objects
        if (NavSys)
        {
            FNavLocation NavLocation;
            if (NavSys->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500, 500, 1000)))
            {
                SpawnLocation = NavLocation.Location;
            }
        }

        FRotator SpawnRotation = FRotator(0, 0, 0);

        // Spawn it
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(EnvClass, SpawnLocation, SpawnRotation, SpawnParams);
        
        if (SpawnedActor)
        {
            TotalSpawned++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("IslandGameMode: Spawned %d environment objects"), TotalSpawned);
}

FVector AIslandGameMode::GetRandomLocationInBounds(const FVector& BoundsMin, const FVector& BoundsMax) const
{
    return FVector(
        FMath::FRandRange(BoundsMin.X, BoundsMax.X),
        FMath::FRandRange(BoundsMin.Y, BoundsMax.Y),
        FMath::FRandRange(BoundsMin.Z, BoundsMax.Z)
    );
}

