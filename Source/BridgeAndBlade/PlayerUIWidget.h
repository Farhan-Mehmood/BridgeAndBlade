// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "PlayerUIWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class APaperChar;

UCLASS()
class BRIDGEANDBLADE_API UPlayerUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Called by the character to update health text
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetHealthText(int NewHealth);

	// Update a quick slot display (pass empty ItemData with ItemName None to clear)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetQuickSlot(int SlotIndex, const FItemData& ItemData);

	// Set owning character so widget can call UseQuickSlot
	void SetOwningCharacter(APaperChar* Character);

protected:
	// Health
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UImage* HealthImage;

	// Quick slot 1
	UPROPERTY(meta = (BindWidget))
	UButton* QuickSlotButton1;

	UPROPERTY(meta = (BindWidget))
	UImage* QuickSlotIcon1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuickSlotName1;

	// Quick slot 2
	UPROPERTY(meta = (BindWidget))
	UButton* QuickSlotButton2;

	UPROPERTY(meta = (BindWidget))
	UImage* QuickSlotIcon2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuickSlotName2;

	// Quick slot 3
	UPROPERTY(meta = (BindWidget))
	UButton* QuickSlotButton3;

	UPROPERTY(meta = (BindWidget))
	UImage* QuickSlotIcon3;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuickSlotName3;

	// Quick slot 4
	UPROPERTY(meta = (BindWidget))
	UButton* QuickSlotButton4;

	UPROPERTY(meta = (BindWidget))
	UImage* QuickSlotIcon4;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuickSlotName4;

	// Quick slot 5
	UPROPERTY(meta = (BindWidget))
	UButton* QuickSlotButton5;

	UPROPERTY(meta = (BindWidget))
	UImage* QuickSlotIcon5;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuickSlotName5;

private:
	APaperChar* OwningCharacter;

	// Internal helper to route button clicks
	UFUNCTION()
	void OnQuickSlotClicked1();

	UFUNCTION()
	void OnQuickSlotClicked2();

	UFUNCTION()
	void OnQuickSlotClicked3();

	UFUNCTION()
	void OnQuickSlotClicked4();

	UFUNCTION()
	void OnQuickSlotClicked5();

	// Helper to set icon safely
	void SetSlotIcon(UImage* IconWidget, UTexture2D* Icon);
};
