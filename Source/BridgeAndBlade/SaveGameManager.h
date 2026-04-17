// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BridgeAndBladeSaveGame.h"
#include "SaveGameManager.generated.h"

class APaperChar;
class ABridgeZone;

/**
 *
 */
UCLASS()
class BRIDGEANDBLADE_API USaveGameManager : public UObject
{
    GENERATED_BODY()

public:
    // Get singleton instance
    UFUNCTION(BlueprintCallable, Category = "Save System")
    static USaveGameManager* Get(UObject* WorldContextObject);

    // Save the game
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool SaveGame(APaperChar* PlayerCharacter, const FString& SlotName = TEXT("PlayerSaveSlot"));

    // Load the game
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool LoadGame(APaperChar* PlayerCharacter, const FString& SlotName = TEXT("PlayerSaveSlot"));

    // Check if a save exists
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DoesSaveExist(const FString& SlotName = TEXT("PlayerSaveSlot"));

    // Delete a save
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DeleteSave(const FString& SlotName = TEXT("PlayerSaveSlot"));

    // Bridge state management
    UFUNCTION(BlueprintCallable, Category = "Save System")
    void RegisterBuiltBridge(FName BridgeID);

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool IsBridgeBuilt(FName BridgeID) const;

    // Get current save data
    UFUNCTION(BlueprintCallable, Category = "Save System")
    UBridgeAndBladeSaveGame* GetCurrentSaveData() const { return CurrentSaveData; }

protected:
    UPROPERTY()
    UBridgeAndBladeSaveGame* CurrentSaveData;

private:
    static USaveGameManager* Instance;
};