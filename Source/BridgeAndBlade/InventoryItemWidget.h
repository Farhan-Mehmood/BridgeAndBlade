// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "InventoryItemWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class APaperChar;

UCLASS()
class BRIDGEANDBLADE_API UInventoryItemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetItemData(const FItemData& Data, int32 Quantity, APaperChar* Character);

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemQuantityText;

    UPROPERTY(meta = (BindWidget))
    UImage* ItemIcon;

    UPROPERTY(meta = (BindWidget))
    UButton* EquipButton;

    UPROPERTY()
    FItemData ItemData;

    UPROPERTY()
    APaperChar* OwningCharacter;

    int32 ItemQuantity;

    UFUNCTION()
    void OnEquipButtonClicked();
};