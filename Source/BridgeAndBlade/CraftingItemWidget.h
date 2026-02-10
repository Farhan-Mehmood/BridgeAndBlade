// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "CraftingItemWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class APaperChar;
class UInventoryWidget;

UCLASS()
class BRIDGEANDBLADE_API UCraftingItemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void SetItemData(const FItemData& Data, APaperChar* Character, UInventoryWidget* InvWidget);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void UpdateCraftability();

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* RequirementsText;

    UPROPERTY(meta = (BindWidget))
    UImage* ItemIcon;

    UPROPERTY(meta = (BindWidget))
    UButton* CraftButton;

    UPROPERTY()
    FItemData ItemData;

    UPROPERTY()
    APaperChar* OwningCharacter;

    UPROPERTY()
    UInventoryWidget* InventoryWidget;

    UFUNCTION()
    void OnCraftButtonClicked();
};