// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "PaperChar.h"
#include "InventoryItemWidget.h"
#include "CraftingItemWidget.h"
#include "ItemDatabase.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseButtonClicked);
    }
}

void UInventoryWidget::SetOwningCharacter(APaperChar* Character)
{
    OwningCharacter = Character;
    RefreshInventory();
    RefreshCraftingList();
}

void UInventoryWidget::RefreshInventory()
{
    if (!InventoryScrollBox || !OwningCharacter || !ItemWidgetClass)
        return;

    // Clear existing items
    InventoryScrollBox->ClearChildren();

    // Add material inventory
    for (const auto& Pair : OwningCharacter->MaterialInventory)
    {
        UInventoryItemWidget* ItemWidget = CreateWidget<UInventoryItemWidget>(this, ItemWidgetClass);
        if (ItemWidget)
        {
            FItemData ItemData;
            if (UItemDatabase::Get(this)->GetItemData(Pair.Key, ItemData))
            {
                ItemWidget->SetItemData(ItemData, Pair.Value, OwningCharacter);
                InventoryScrollBox->AddChild(ItemWidget);
            }
        }
    }

    // Add weapon inventory
    for (const FName& WeaponName : OwningCharacter->WeaponInventory)
    {
        UInventoryItemWidget* ItemWidget = CreateWidget<UInventoryItemWidget>(this, ItemWidgetClass);
        if (ItemWidget)
        {
            FItemData ItemData;
            if (UItemDatabase::Get(this)->GetItemData(WeaponName, ItemData))
            {
                ItemWidget->SetItemData(ItemData, 1, OwningCharacter);
                InventoryScrollBox->AddChild(ItemWidget);
            }
        }
    }
}

void UInventoryWidget::RefreshCraftingList()
{
    if (!CraftingScrollBox || !OwningCharacter || !CraftingWidgetClass)
        return;

    // Clear existing items
    CraftingScrollBox->ClearChildren();

    // Get all craftable items
    TArray<FItemData> CraftableItems = UItemDatabase::Get(this)->GetCraftableItems();

    for (const FItemData& ItemData : CraftableItems)
    {
        UCraftingItemWidget* CraftWidget = CreateWidget<UCraftingItemWidget>(this, CraftingWidgetClass);
        if (CraftWidget)
        {
            CraftWidget->SetItemData(ItemData, OwningCharacter, this);
            CraftingScrollBox->AddChild(CraftWidget);
        }
    }
}

void UInventoryWidget::OnCloseButtonClicked()
{
    if (OwningCharacter)
    {
        OwningCharacter->ToggleInventory();
    }
}