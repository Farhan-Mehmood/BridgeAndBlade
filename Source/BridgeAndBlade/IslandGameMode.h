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

    // Enemy spawning
    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    TArray<TSubclassOf<APaperEnemy>> EnemyClasses;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    int32 TotalEnemiesToSpawn = 15;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    FVector EnemyBoundsMin = FVector(-5000, -5000, 0);

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Enemies")
    FVector EnemyBoundsMax = FVector(5000, 5000, 0);

    // Environment spawning
    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    TArray<TSubclassOf<AActor>> EnvironmentActorClasses;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    int32 EnvironmentObjectsToSpawn = 20;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    FVector IslandBoundsMin = FVector(-5000, -5000, 0);

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Environment")
    FVector IslandBoundsMax = FVector(5000, 5000, 0);

private:
    void SpawnEnemies();
    void SpawnEnvironmentObjects();
    
    FVector GetRandomLocationInBounds(const FVector& BoundsMin, const FVector& BoundsMax) const;
};
