// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Material UMETA(DisplayName = "Crafting Material"),
    Weapon UMETA(DisplayName = "Weapon"),
    Placeable UMETA(DisplayName = "Placeable Object"),
    Consumable UMETA(DisplayName = "Consumable")
};

USTRUCT(BlueprintType)
struct FCraftingRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int Amount;

    FCraftingRequirement()
        : ItemName(NAME_None), Amount(0)
    {}

    FCraftingRequirement(FName InName, int InAmount)
        : ItemName(InName), Amount(InAmount)
    {}
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FName ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    int MaxStackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FCraftingRequirement> CraftingRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bIsCraftable;

    // For weapons
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::Weapon"))
    TSubclassOf<class AWeaponBase> WeaponClass;

    // For placeables
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placeable", meta = (EditCondition = "ItemType == EItemType::Placeable"))
    TSubclassOf<class AActor> PlaceableClass;

    FItemData()
        : ItemName(NAME_None)
        , DisplayName(FText::FromString("Unknown Item"))
        , Description(FText::FromString("No description"))
        , ItemType(EItemType::Material)
        , Icon(nullptr)
        , MaxStackSize(99)
        , bIsCraftable(false)
        , WeaponClass(nullptr)
        , PlaceableClass(nullptr)
    {
    }
};