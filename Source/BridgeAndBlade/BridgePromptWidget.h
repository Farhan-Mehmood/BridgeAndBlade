// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BridgePromptWidget.generated.h"

class UTextBlock;
class UButton;
class ABridgeZone;

UCLASS()
class BRIDGEANDBLADE_API UBridgePromptWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Bridge")
    void SetBridgeZone(ABridgeZone* Zone);

    UFUNCTION(BlueprintCallable, Category = "Bridge")
    void UpdatePrompt();

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TitleText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DescriptionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CostText;

    UPROPERTY(meta = (BindWidget))
    UButton* YesButton;

    UPROPERTY(meta = (BindWidget))
    UButton* NoButton;

    UPROPERTY()
    ABridgeZone* BridgeZone;

    UFUNCTION()
    void OnYesButtonClicked();

    UFUNCTION()
    void OnNoButtonClicked();
};