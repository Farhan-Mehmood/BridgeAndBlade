// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
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
    RefreshEquipment();
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
    TMap<FName, int32> WeaponCounts;
    for (const FName& WeaponName : OwningCharacter->WeaponInventory)
    {
        WeaponCounts.FindOrAdd(WeaponName)++;
    }

    for (const auto& Pair : WeaponCounts)
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

    
    RefreshEquipment();
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

void UInventoryWidget::RefreshEquipment()
{
    if (!OwningCharacter) return;

    // Update Stats Text
    if (DefenseStatText)
    {
        FString DefenseString = FString::Printf(TEXT("Defense: %d"), FMath::RoundToInt(OwningCharacter->TotalDefense));
        DefenseStatText->SetText(FText::FromString(DefenseString));
    }

    if (AttackStatText)
    {
        FString AttackString = FString::Printf(TEXT("Attack: %d"), FMath::RoundToInt(OwningCharacter->TotalAttack));
        AttackStatText->SetText(FText::FromString(AttackString));
    }

    // Update Equipped Armor Slots using standard UI Images
    UItemDatabase* DB = UItemDatabase::Get(this);
    if (!DB) return;

    // Helper lambda to update an Image component
    auto UpdateSlotImage = [this, DB](EArmorSlot ArmorSlotId, UImage* SlotImage)
    {
        if (!SlotImage) return;

        if (OwningCharacter->EquippedArmor.Contains(ArmorSlotId))
        {
            FName EquippedItemName = OwningCharacter->EquippedArmor[ArmorSlotId];
            FItemData ItemData;
            
            // If item has a valid texture icon, apply it
            if (DB->GetItemData(EquippedItemName, ItemData) && ItemData.Icon != nullptr)
            {
                SlotImage->SetBrushFromTexture(ItemData.Icon);
                SlotImage->SetVisibility(ESlateVisibility::Visible); // Show the icon
                return;
            }
        }
        
        // If there is no item equipped, or the item has no icon -> Hide the image
        SlotImage->SetVisibility(ESlateVisibility::Hidden);
    };

    UpdateSlotImage(EArmorSlot::Head, HeadSlotIcon);
    UpdateSlotImage(EArmorSlot::Chest, ChestSlotIcon);
    UpdateSlotImage(EArmorSlot::Legs, LegsSlotIcon);
    UpdateSlotImage(EArmorSlot::Boots, BootsSlotIcon);
}

void UInventoryWidget::OnCloseButtonClicked()
{
    if (OwningCharacter)
    {
        OwningCharacter->ToggleInventory();
    }
}