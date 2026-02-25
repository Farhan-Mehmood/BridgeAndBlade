// Fill out your copyright notice in the Description page of Project Settings.

#include "IslandGameMode.h"
#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"

AIslandGameMode::AIslandGameMode()
{
    // Don't set DefaultPawnClass here - let it be configured in Blueprint
}

void AIslandGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Give the level a moment to fully load before spawning
    FTimerHandle SpawnTimer;
    GetWorld()->GetTimerManager().SetTimer(SpawnTimer, [this]()
    {
        SpawnEnemies();
        SpawnEnvironmentObjects();
    }, 0.5f, false);
}

void AIslandGameMode::SpawnEnemies()
{
    if (EnemyClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("IslandGameMode: No enemy classes configured for spawning"));
        return;
    }

    // // Look for TargetPoint actors placed in the level to use as spawn markers
    // TArray<AActor*> SpawnPoints;
    // UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), SpawnPoints);

    // if (SpawnPoints.Num() == 0)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("IslandGameMode: No spawn points found. Add TargetPoint actors to your level."));
    //     return;
    // }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    int32 TotalSpawned = 0;
    // TotalEnemiesToSpawn is now set directly from the property

    // Spawn enemies randomly across the defined bounds
    for (int32 i = 0; i < TotalEnemiesToSpawn; ++i)
    {
        // Pick a random enemy type
        TSubclassOf<APaperEnemy> EnemyClass = EnemyClasses[FMath::RandRange(0, EnemyClasses.Num() - 1)];

        // Get a random location within enemy spawn bounds
        FVector SpawnLocation = GetRandomLocationInBounds(EnemyBoundsMin, EnemyBoundsMax);

        // Snap to navmesh so they can actually move
        if (NavSys)
        {
            FNavLocation NavLocation;
            if (NavSys->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500, 500, 500)))
            {
                SpawnLocation = NavLocation.Location;
            }
            else
            {
                // Skip this spawn if we can't find navmesh
                UE_LOG(LogTemp, Warning, TEXT("IslandGameMode: Couldn't project spawn location to navmesh, skipping"));
                continue;
            }
        }

        // Actually spawn the enemy
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APaperEnemy* SpawnedEnemy = GetWorld()->SpawnActor<APaperEnemy>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        
        if (SpawnedEnemy)
        {
            TotalSpawned++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("IslandGameMode: Spawned %d enemies randomly across island"), TotalSpawned);
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

        // Random rotation for variety
        FRotator SpawnRotation = FRotator(0, FMath::FRandRange(0.0f, 360.0f), 0);

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

