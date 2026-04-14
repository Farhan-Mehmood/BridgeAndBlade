// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "InventoryWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UImage;
class APaperChar;

/*
 * 
 */
UCLASS()
class BRIDGEANDBLADE_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Set the owning player character
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetOwningCharacter(APaperChar* Character);

    // Refresh the inventory display
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshInventory();

    // Refresh the crafting list
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshCraftingList();

    // Refresh equipment (armor slots and stats)
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    void RefreshEquipment();

protected:
    UPROPERTY()
    APaperChar* OwningCharacter;

    // Widget bindings
    UPROPERTY(meta = (BindWidget))
    UScrollBox* InventoryScrollBox;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* CraftingScrollBox;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    //  Stat Displays
    UPROPERTY(meta = (BindWidgetOptional)) 
    UTextBlock* DefenseStatText;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* AttackStatText;

    // Armor Slots (Optional but recommended for visual feedback)
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* HeadSlotIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* ChestSlotIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* LegsSlotIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* BootsSlotIcon;

    // Item entry widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UInventoryItemWidget> ItemWidgetClass;

    // Crafting entry widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI") 
    TSubclassOf<class UCraftingItemWidget> CraftingWidgetClass;


    UFUNCTION()
    void OnCloseButtonClicked();
};