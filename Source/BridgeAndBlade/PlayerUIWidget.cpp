// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerUIWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "PaperChar.h"
#include "ItemDatabase.h"

void UPlayerUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (QuickSlotButton1) QuickSlotButton1->OnClicked.AddDynamic(this, &UPlayerUIWidget::OnQuickSlotClicked1);
	if (QuickSlotButton2) QuickSlotButton2->OnClicked.AddDynamic(this, &UPlayerUIWidget::OnQuickSlotClicked2);
	if (QuickSlotButton3) QuickSlotButton3->OnClicked.AddDynamic(this, &UPlayerUIWidget::OnQuickSlotClicked3);
	if (QuickSlotButton4) QuickSlotButton4->OnClicked.AddDynamic(this, &UPlayerUIWidget::OnQuickSlotClicked4);
	if (QuickSlotButton5) QuickSlotButton5->OnClicked.AddDynamic(this, &UPlayerUIWidget::OnQuickSlotClicked5);
}

void UPlayerUIWidget::SetOwningCharacter(APaperChar* Character)
{
	OwningCharacter = Character;
}

void UPlayerUIWidget::SetHealthText(int NewHealth)
{
	if (HealthText)
	{
		HealthText->SetText(FText::AsNumber(NewHealth));
	}
}

void UPlayerUIWidget::SetQuickSlot(int SlotIndex, const FItemData& ItemData)
{
	UImage* IconWidget = nullptr;
	UTextBlock* NameWidget = nullptr;

	switch (SlotIndex)
	{
	case 0:
		IconWidget = QuickSlotIcon1;
		NameWidget = QuickSlotName1;
		break;
	case 1:
		IconWidget = QuickSlotIcon2;
		NameWidget = QuickSlotName2;
		break;
	case 2:
		IconWidget = QuickSlotIcon3;
		NameWidget = QuickSlotName3;
		break;
	case 3:
		IconWidget = QuickSlotIcon4;
		NameWidget = QuickSlotName4;
		break;
	case 4:
		IconWidget = QuickSlotIcon5;
		NameWidget = QuickSlotName5;
		break;
	default:
		return;
	}

	// If ItemName is None, clear slot
	if (ItemData.ItemName.IsNone())
	{
		if (NameWidget) NameWidget->SetText(FText::GetEmpty());
		SetSlotIcon(IconWidget, nullptr);
		return;
	}

	// Set display name and icon (icon may be null)
	if (NameWidget)
	{
		NameWidget->SetText(ItemData.DisplayName);
	}

	if (IconWidget)
	{
		SetSlotIcon(IconWidget, ItemData.Icon);
	}
}

void UPlayerUIWidget::SetSlotIcon(UImage* IconWidget, UTexture2D* Icon)
{
	if (!IconWidget)
		return;

	if (Icon)
	{
		IconWidget->SetBrushFromTexture(Icon);
		IconWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		IconWidget->SetBrushFromTexture(nullptr);
		IconWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerUIWidget::OnQuickSlotClicked1()
{
	if (OwningCharacter) OwningCharacter->UseQuickSlot(0);
}

void UPlayerUIWidget::OnQuickSlotClicked2()
{
	if (OwningCharacter) OwningCharacter->UseQuickSlot(1);
}

void UPlayerUIWidget::OnQuickSlotClicked3()
{
	if (OwningCharacter) OwningCharacter->UseQuickSlot(2);
}

void UPlayerUIWidget::OnQuickSlotClicked4()
{
	if (OwningCharacter) OwningCharacter->UseQuickSlot(3);
}

void UPlayerUIWidget::OnQuickSlotClicked5()
{
	if (OwningCharacter) OwningCharacter->UseQuickSlot(4);
}

