// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemDatabase.h"
#include "Engine/DataTable.h"

UItemDatabase* UItemDatabase::Instance = nullptr;

UItemDatabase* UItemDatabase::Get(UObject* WorldContextObject)
{
    if (!Instance)
    {
        Instance = NewObject<UItemDatabase>();
        Instance->AddToRoot(); // Prevent garbage collection

        UE_LOG(LogTemp, Log, TEXT("ItemDatabase singleton created"));
    }
    return Instance;
}

void UItemDatabase::Initialize(UDataTable* ItemDataTable)
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("ItemDatabase::Initialize - DataTable is null!"));
        return;
    }

    ItemTable = ItemDataTable;
    CacheItemData();

    UE_LOG(LogTemp, Log, TEXT("ItemDatabase initialized with %d items"), ItemCache.Num());
}

void UItemDatabase::CacheItemData()
{
    if (!ItemTable)
        return;

    ItemCache.Empty();

    TArray<FName> RowNames = ItemTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FItemData* ItemData = ItemTable->FindRow<FItemData>(RowName, TEXT("ItemDatabase"));
        if (ItemData)
        {
            // Use the row name as the item name if not specified
            if (ItemData->ItemName.IsNone())
            {
                ItemData->ItemName = RowName;
            }

            ItemCache.Add(ItemData->ItemName, *ItemData);
        }
    }
}

bool UItemDatabase::GetItemData(FName ItemName, FItemData& OutItemData) const
{
    if (ItemCache.Contains(ItemName))
    {
        OutItemData = ItemCache[ItemName];
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Item '%s' not found in database"), *ItemName.ToString());
    return false;
}

bool UItemDatabase::HasItem(FName ItemName) const
{
    return ItemCache.Contains(ItemName);
}

TArray<FItemData> UItemDatabase::GetItemsByType(EItemType ItemType) const
{
    TArray<FItemData> Result;

    for (const auto& Pair : ItemCache)
    {
        if (Pair.Value.ItemType == ItemType)
        {
            Result.Add(Pair.Value);
        }
    }

    return Result;
}

TArray<FItemData> UItemDatabase::GetCraftableItems() const
{
    TArray<FItemData> Result;

    for (const auto& Pair : ItemCache)
    {
        if (Pair.Value.bIsCraftable)
        {
            Result.Add(Pair.Value);
        }
    }

    return Result;
}

bool UItemDatabase::CanCraftItem(FName ItemName, const TMap<FName, int>& AvailableMaterials) const
{
    FItemData ItemData;
    if (!GetItemData(ItemName, ItemData))
    {
        return false;
    }

    if (!ItemData.bIsCraftable)
    {
        return false;
    }

    // Check if we have all required materials
    for (const FCraftingRequirement& Requirement : ItemData.CraftingRequirements)
    {
        if (!AvailableMaterials.Contains(Requirement.ItemName))
        {
            return false;
        }

        if (AvailableMaterials[Requirement.ItemName] < Requirement.Amount)
        {
            return false;
        }
    }

    return true;
}

TArray<FName> UItemDatabase::GetAllItemNames() const
{
    TArray<FName> Names;
    ItemCache.GetKeys(Names);
    return Names;
}