// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "PaperChar.h"
#include "InventoryWidget.h"

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EquipButton)
	{
		EquipButton->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnEquipButtonClicked);
	}
	if (DeleteButton)
	{
		DeleteButton->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnDeleteButtonClicked);
	}
	// Bind new assign button if present in the widget
	if (AssignButton)
	{
		AssignButton->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnAssignButtonClicked);
	}
}

void UInventoryItemWidget::SetItemData(const FItemData& Data, int Quantity, APaperChar* Character)
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

	// Show equip button for both Weapons AND Armor
	if (EquipButton)
	{
		bool bCanEquip = (Data.ItemType == EItemType::Weapon || Data.ItemType == EItemType::Armor);
		EquipButton->SetVisibility(bCanEquip ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Hide assign button for Armor
	if (AssignButton)
	{
		bool bCanAssign = (Data.ItemType != EItemType::Armor);
		AssignButton->SetVisibility(bCanAssign ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UInventoryItemWidget::OnEquipButtonClicked()
{
	if (!OwningCharacter)
		return;

	if (ItemData.ItemType == EItemType::Weapon)
	{
		// Find the weapon in inventory and equip it
		for (int i = 0; i < OwningCharacter->WeaponInventory.Num(); ++i)
		{
			if (OwningCharacter->WeaponInventory[i] == ItemData.ItemName)
			{
				OwningCharacter->EquipWeapon(i);
				break;
			}
		}
	}
	else if (ItemData.ItemType == EItemType::Armor)
	{
		// Equip the armor piece
		OwningCharacter->EquipArmor(ItemData.ItemName);

		// Toggle the inventory so it will update automatically.
		if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(GetOuter()))
		 {
			 InvWidget->RefreshInventory();
			 InvWidget->RefreshEquipment();
		}
	}
}

void UInventoryItemWidget::OnDeleteButtonClicked()
{
	if (!OwningCharacter)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Delete button clicked for item: %s"), *ItemData.ItemName.ToString());

	// Try to remove the item from the character.
	bool bWasRemoved = OwningCharacter->RemoveItem(ItemData.ItemName, 1);
	
	if (bWasRemoved)
	{
		if (ItemData.ItemType == EItemType::Weapon)
		{
			OwningCharacter->UnequipWeapon();
		}

		// Update the UI to reflect the change
		int32 NewQuantity = ItemQuantity - 1;

		if (NewQuantity <= 0)
		{
			// Try removing from quick slots since the player has none left
			bool bQuickSlotUpdated = false;
			for (int i = 0; i < OwningCharacter->QuickSlots.Num(); ++i)
			{
				if (OwningCharacter->QuickSlots[i] == ItemData.ItemName)
				{
					OwningCharacter->QuickSlots[i] = NAME_None;
					bQuickSlotUpdated = true;
				}
			}

			// If we unassigned a quick slot, refresh that specific UI part
			if (bQuickSlotUpdated)
			{
				OwningCharacter->RefreshQuickSlots();
			}

			// Eliminate the UI element completely from the inventory list when we reach 0
			RemoveFromParent();
		}
		else
		{
			SetItemData(ItemData, NewQuantity, OwningCharacter);
		}
	}
}

void UInventoryItemWidget::OnAssignButtonClicked()
{
	if (!OwningCharacter)
		return;

	// Find first empty quick slot, otherwise overwrite slot 0
	int ChosenSlot = INDEX_NONE;
	for (int i = 0; i < OwningCharacter->QuickSlots.Num(); ++i)
	{
		if (OwningCharacter->QuickSlots[i].IsNone())
		{
			ChosenSlot = i;
			break;
		}
	}
	if (ChosenSlot == INDEX_NONE && OwningCharacter->QuickSlots.Num() > 0)
	{
		ChosenSlot = 0;
	}

	if (ChosenSlot != INDEX_NONE)
	{
		OwningCharacter->AssignQuickSlot(ChosenSlot, ItemData.ItemName);
		UE_LOG(LogTemp, Log, TEXT("Assigned item %s to quick slot %d"), *ItemData.ItemName.ToString(), ChosenSlot + 1);
	}
}