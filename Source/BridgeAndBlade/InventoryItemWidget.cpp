// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "PaperChar.h"

void UInventoryItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (EquipButton)
    {
        EquipButton->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnEquipButtonClicked);
    }
}

void UInventoryItemWidget::SetItemData(const FItemData& Data, int32 Quantity, APaperChar* Character)
{
    ItemData = Data;
    ItemQuantity = Quantity;
    OwningCharacter = Character;

    if (ItemNameText)
    {
        ItemNameText->SetText(Data.DisplayName);
    }

    if (ItemQuantityText)
    {
        ItemQuantityText->SetText(FText::AsNumber(Quantity));
    }

    if (ItemIcon && Data.Icon)
    {
        ItemIcon->SetBrushFromTexture(Data.Icon);
    }

    // Hide equip button for non-weapons
    if (EquipButton)
    {
        EquipButton->SetVisibility(
            Data.ItemType == EItemType::Weapon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
        );
    }
}

void UInventoryItemWidget::OnEquipButtonClicked()
{
    if (!OwningCharacter || ItemData.ItemType != EItemType::Weapon)
        return;

    // Find the weapon in inventory and equip it
    for (int32 i = 0; i < OwningCharacter->WeaponInventory.Num(); ++i)
    {
        if (OwningCharacter->WeaponInventory[i] == ItemData.ItemName)
        {
            OwningCharacter->EquipWeapon(i);
            break;
        }
    }
}