// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "PaperChar.h"
#include "InventoryWidget.h"

void UCraftingItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CraftButton)
    {
        CraftButton->OnClicked.AddDynamic(this, &UCraftingItemWidget::OnCraftButtonClicked);
    }
}

void UCraftingItemWidget::SetItemData(const FItemData& Data, APaperChar* Character, UInventoryWidget* InvWidget)
{
    ItemData = Data;
    OwningCharacter = Character;
    InventoryWidget = InvWidget;

    if (ItemNameText)
    {
        ItemNameText->SetText(Data.DisplayName);
    }

    if (ItemIcon && Data.Icon)
    {
        ItemIcon->SetBrushFromTexture(Data.Icon);
    }

    // Build requirements text
    if (RequirementsText)
    {
        FString ReqText = "Requires: ";
        for (int i = 0; i < Data.CraftingRequirements.Num(); ++i)
        {
            const FCraftingRequirement& Req = Data.CraftingRequirements[i];
            ReqText += FString::Printf(TEXT("%s x%d"), *Req.ItemName.ToString(), Req.Amount);

            if (i < Data.CraftingRequirements.Num() - 1)
            {
                ReqText += ", ";
            }
        }
        RequirementsText->SetText(FText::FromString(ReqText));
    }

    UpdateCraftability();
}

void UCraftingItemWidget::UpdateCraftability()
{
    if (!CraftButton || !OwningCharacter)
        return;

    bool bCanCraft = OwningCharacter->CanCraftItem(ItemData.ItemName);
    CraftButton->SetIsEnabled(bCanCraft);

    // Change button color based on craftability
    if (bCanCraft)
    {
        CraftButton->SetBackgroundColor(FLinearColor::Green);
    }
    else
    {
        CraftButton->SetBackgroundColor(FLinearColor::Red);
    }
}

void UCraftingItemWidget::OnCraftButtonClicked()
{
    if (!OwningCharacter)
        return;

    if (OwningCharacter->CraftItem(ItemData.ItemName))
    {
        // Refresh the inventory display
        if (InventoryWidget)
        {
            InventoryWidget->RefreshInventory();
            InventoryWidget->RefreshCraftingList();
        }
    }
}