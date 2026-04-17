// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ItemData.h"
#include "BridgeAndBladeSaveGame.generated.h"

/**
 * Save game data structure
 */
UCLASS()
class BRIDGEANDBLADE_API UBridgeAndBladeSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UBridgeAndBladeSaveGame();

    // Save slot name
    UPROPERTY()
    FString SaveSlotName;

    UPROPERTY()
    int UserIndex;

    // Player Data
    UPROPERTY()
    FVector PlayerLocation;

    UPROPERTY()
    FRotator PlayerRotation;

    UPROPERTY()
    FString CurrentLevelName;

    UPROPERTY()
    int PlayerHealth;

    // Inventory Data
    UPROPERTY()
    TMap<FName, int> MaterialInventory;

    UPROPERTY()
    TArray<FName> WeaponInventory;

    UPROPERTY()
    FName EquippedWeaponName;

    // Quick Slots (Hotbar)
    UPROPERTY()
    TArray<FName> QuickSlots;

    // Equipped Armor
    UPROPERTY()
    TMap<uint8, FName> EquippedArmorMap; // uint8 maps to EArmorSlot

    // Bridge States - Key is bridge unique ID, Value is whether it's built
    UPROPERTY()
    TMap<FName, bool> BuiltBridges;

    // Stats
    UPROPERTY()
    float BaseDefense;

    UPROPERTY()
    float BaseAttack;
};