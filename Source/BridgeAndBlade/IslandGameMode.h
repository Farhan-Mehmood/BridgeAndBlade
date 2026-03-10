// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IslandGameMode.generated.h"

class APaperEnemy;

UCLASS()
class BRIDGEANDBLADE_API AIslandGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AIslandGameMode();

protected:
    virtual void BeginPlay() override;

    // Enemy spawning: available enemy classes
    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    TArray<TSubclassOf<APaperEnemy>> EnemyClasses;

    // Maximum number of concurrently spawned enemies
    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    int32 MaxConcurrentEnemies = 12;

    // How often to attempt spawning (seconds)
    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    float SpawnIntervalSeconds = 2.0f;

    // Minimum and maximum radius from the player where enemies will spawn (world units)
    UPROPERTY(EditAnywhere, Category = "Spawning|Enemies")
    float SpawnMinRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Spawning|Enemies")
    float SpawnMaxRadius = 2200.0f;

    // If an enemy is further than this from the player it will be despawned (world units)
    UPROPERTY(EditAnywhere, Category = "Spawning|Enemies")
    float DespawnRadius = 3000.0f;

    // Environment spawning (unchanged)
    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    TArray<TSubclassOf<AActor>> EnvironmentActorClasses;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    int32 EnvironmentObjectsToSpawn = 20;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    FVector IslandBoundsMin = FVector(-5000, -5000, 0);

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    FVector IslandBoundsMax = FVector(5000, 5000, 0);

private:
    // Timer for repeated spawning
    FTimerHandle SpawnTimerHandle;

    // Active spawned enemies tracked here
    UPROPERTY()
    TArray<APaperEnemy*> SpawnedEnemies;

    // Spawns one enemy near the player (inside spawn ring). Respects MaxConcurrentEnemies.
    void TrySpawnTick();

    // Despawn enemies that are far from the player or invalid
    void CleanupFarEnemies();

    // Helper to compute a random point in the ring around the player
    FVector GetRandomPointAroundPlayer(float MinRadius, float MaxRadius) const;

    // Environment spawn (unchanged)
    void SpawnEnvironmentObjects();

    FVector GetRandomLocationInBounds(const FVector& BoundsMin, const FVector& BoundsMax) const;
};
