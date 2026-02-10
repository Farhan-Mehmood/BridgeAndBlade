// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "ItemData.h"
#include "ItemDatabase.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class BRIDGEANDBLADE_API UItemDatabase : public UObject
{
    GENERATED_BODY()

public:
    // Get the singleton instance
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    static UItemDatabase* Get(UObject* WorldContextObject);

    // Initialize the database with a data table
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    void Initialize(UDataTable* ItemDataTable);

    // Get item data by name
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    bool GetItemData(FName ItemName, FItemData& OutItemData) const;

    // Check if an item exists
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    bool HasItem(FName ItemName) const;

    // Get all items of a specific type
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    TArray<FItemData> GetItemsByType(EItemType ItemType) const;

    // Get all craftable items
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    TArray<FItemData> GetCraftableItems() const;

    // Validate if crafting requirements can be met
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    bool CanCraftItem(FName ItemName, const TMap<FName, int>& AvailableMaterials) const;

    // Get all item names
    UFUNCTION(BlueprintCallable, Category = "Item Database")
    TArray<FName> GetAllItemNames() const;

protected:
    UPROPERTY()
    UDataTable* ItemTable;

    UPROPERTY()
    TMap<FName, FItemData> ItemCache;

private:
    static UItemDatabase* Instance;
    void CacheItemData();
};
