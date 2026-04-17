// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameManager.h"

#include "SaveGameManager.h"
#include "PaperChar.h"
#include "BridgeZone.h"
#include "Kismet/GameplayStatics.h"

USaveGameManager* USaveGameManager::Instance = nullptr;

USaveGameManager* USaveGameManager::Get(UObject* WorldContextObject)
{
    if (!Instance)
    {
        Instance = NewObject<USaveGameManager>();
        Instance->AddToRoot(); // Prevent garbage collection
        Instance->CurrentSaveData = nullptr;

        UE_LOG(LogTemp, Log, TEXT("SaveGameManager singleton created"));
    }
    return Instance;
}

bool USaveGameManager::SaveGame(APaperChar* PlayerCharacter, const FString& SlotName)
{
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot save: PlayerCharacter is null"));
        return false;
    }

    // Create save game object
    UBridgeAndBladeSaveGame* SaveGameInstance = Cast<UBridgeAndBladeSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UBridgeAndBladeSaveGame::StaticClass())
    );

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create save game object"));
        return false;
    }

    // Save slot info
    SaveGameInstance->SaveSlotName = SlotName;
    SaveGameInstance->UserIndex = 0;

    // Save player location and rotation
    SaveGameInstance->PlayerLocation = PlayerCharacter->GetActorLocation();
    SaveGameInstance->PlayerRotation = PlayerCharacter->GetActorRotation();

    // Save current level name
    UWorld* World = PlayerCharacter->GetWorld();
    if (World)
    {
        SaveGameInstance->CurrentLevelName = World->GetMapName();
        // Remove "UEDPIE_0_" prefix if in PIE mode
        SaveGameInstance->CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
    }

    // Save health
    SaveGameInstance->PlayerHealth = PlayerCharacter->health;

    // Save inventory
    SaveGameInstance->MaterialInventory = PlayerCharacter->MaterialInventory;
    SaveGameInstance->WeaponInventory = PlayerCharacter->WeaponInventory;
    SaveGameInstance->EquippedWeaponName = PlayerCharacter->EquippedWeaponName;

    // Save quick slots
    SaveGameInstance->QuickSlots = PlayerCharacter->QuickSlots;

    // Save equipped armor (convert enum to uint8 for serialization)
    SaveGameInstance->EquippedArmorMap.Empty();
    for (const auto& ArmorPair : PlayerCharacter->EquippedArmor)
    {
        SaveGameInstance->EquippedArmorMap.Add(static_cast<uint8>(ArmorPair.Key), ArmorPair.Value);
    }

    // Save stats
    SaveGameInstance->BaseDefense = PlayerCharacter->BaseDefense;
    SaveGameInstance->BaseAttack = PlayerCharacter->BaseAttack;

    // Save bridge states (use current save data if available)
    if (CurrentSaveData)
    {
        SaveGameInstance->BuiltBridges = CurrentSaveData->BuiltBridges;
    }

    // Perform the save
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);

    if (bSuccess)
    {
        CurrentSaveData = SaveGameInstance;
        UE_LOG(LogTemp, Log, TEXT("Game saved successfully to slot: %s"), *SlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
    }

    return bSuccess;
}

bool USaveGameManager::LoadGame(APaperChar* PlayerCharacter, const FString& SlotName)
{
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot load: PlayerCharacter is null"));
        return false;
    }

    // Load save game
    UBridgeAndBladeSaveGame* LoadedGame = Cast<UBridgeAndBladeSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("No save game found in slot: %s"), *SlotName);
        return false;
    }

    CurrentSaveData = LoadedGame;

    // Restore player location and rotation
    PlayerCharacter->SetActorLocation(LoadedGame->PlayerLocation);
    PlayerCharacter->SetActorRotation(LoadedGame->PlayerRotation);

    // Restore health
    PlayerCharacter->health = LoadedGame->PlayerHealth;

    // Restore inventory
    PlayerCharacter->MaterialInventory = LoadedGame->MaterialInventory;
    PlayerCharacter->WeaponInventory = LoadedGame->WeaponInventory;

    // Restore quick slots
    PlayerCharacter->QuickSlots = LoadedGame->QuickSlots;
    PlayerCharacter->RefreshQuickSlots();

    // Restore equipped armor (convert uint8 back to enum)
    PlayerCharacter->EquippedArmor.Empty();
    for (const auto& ArmorPair : LoadedGame->EquippedArmorMap)
    {
        PlayerCharacter->EquippedArmor.Add(static_cast<EArmorSlot>(ArmorPair.Key), ArmorPair.Value);
    }

    // Restore stats
    PlayerCharacter->BaseDefense = LoadedGame->BaseDefense;
    PlayerCharacter->BaseAttack = LoadedGame->BaseAttack;
    PlayerCharacter->RecalculateStats();

    // Restore equipped weapon
    if (!LoadedGame->EquippedWeaponName.IsNone())
    {
        // Find weapon in inventory
        for (int i = 0; i < PlayerCharacter->WeaponInventory.Num(); ++i)
        {
            if (PlayerCharacter->WeaponInventory[i] == LoadedGame->EquippedWeaponName)
            {
                PlayerCharacter->EquipWeapon(i);
                break;
            }
        }
    }

    // Update UI
  //  if (PlayerCharacter->PlayerUIWidget)
  //  {
  //      PlayerCharacter->PlayerUIWidget->SetHealthText(PlayerCharacter->health);
  //  }

    UE_LOG(LogTemp, Log, TEXT("Game loaded successfully from slot: %s"), *SlotName);
    return true;
}

bool USaveGameManager::DoesSaveExist(const FString& SlotName)
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

bool USaveGameManager::DeleteSave(const FString& SlotName)
{
    bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);

    if (bSuccess)
    {
        CurrentSaveData = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Save deleted: %s"), *SlotName);
    }

    return bSuccess;
}

void USaveGameManager::RegisterBuiltBridge(FName BridgeID)
{
    if (!CurrentSaveData)
    {
        // Create a temporary save data if none exists
        CurrentSaveData = Cast<UBridgeAndBladeSaveGame>(
            UGameplayStatics::CreateSaveGameObject(UBridgeAndBladeSaveGame::StaticClass())
        );
    }

    if (CurrentSaveData)
    {
        CurrentSaveData->BuiltBridges.Add(BridgeID, true);
        UE_LOG(LogTemp, Log, TEXT("Bridge registered as built: %s"), *BridgeID.ToString());
    }
}

bool USaveGameManager::IsBridgeBuilt(FName BridgeID) const
{
    if (!CurrentSaveData)
        return false;

    return CurrentSaveData->BuiltBridges.Contains(BridgeID) && CurrentSaveData->BuiltBridges[BridgeID];
}