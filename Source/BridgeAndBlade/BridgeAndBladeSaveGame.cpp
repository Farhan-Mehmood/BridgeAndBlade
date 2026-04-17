// Fill out your copyright notice in the Description page of Project Settings.

#include "BridgeAndBladeSaveGame.h"

UBridgeAndBladeSaveGame::UBridgeAndBladeSaveGame()
{
    SaveSlotName = TEXT("PlayerSaveSlot");
    UserIndex = 0;
    PlayerHealth = 100;
    BaseDefense = 0.0f;
    BaseAttack = 1.0f;
}