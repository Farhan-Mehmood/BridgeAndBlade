// Fill out your copyright notice in the Description page of Project Settings.


#include "BridgePromptWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BridgeZone.h"
#include "ItemData.h"

void UBridgePromptWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (YesButton)
    {
        YesButton->OnClicked.AddDynamic(this, &UBridgePromptWidget::OnYesButtonClicked);
    }

    if (NoButton)
    {
        NoButton->OnClicked.AddDynamic(this, &UBridgePromptWidget::OnNoButtonClicked);
    }
}

void UBridgePromptWidget::SetBridgeZone(ABridgeZone* Zone)
{
    BridgeZone = Zone;
}

void UBridgePromptWidget::UpdatePrompt()
{
    if (!BridgeZone)
        return;

    if (BridgeZone->BridgeState == EBridgeZoneState::NeedsToBuild)
    {
        // Show build prompt
        if (TitleText)
        {
            TitleText->SetText(FText::FromString("Build Bridge?"));
        }

        if (DescriptionText)
        {
            DescriptionText->SetText(FText::FromString("Would you like to build a bridge to the next island?"));
        }

        // Build cost string
        if (CostText)
        {
            FString CostString = "Cost: ";
            for (int32 i = 0; i < BridgeZone->BuildingCost.Num(); ++i)
            {
                const FCraftingRequirement& Cost = BridgeZone->BuildingCost[i];
                CostString += FString::Printf(TEXT("%s x%d"), *Cost.ItemName.ToString(), Cost.Amount);

                if (i < BridgeZone->BuildingCost.Num() - 1)
                {
                    CostString += ", ";
                }
            }
            CostText->SetText(FText::FromString(CostString));

            // Check if player can afford
            if (!BridgeZone->CanPlayerAffordBridge())
            {
                CostText->SetColorAndOpacity(FLinearColor::Red);
            }
            else
            {
                CostText->SetColorAndOpacity(FLinearColor::Green);
            }
        }

        // Enable/disable yes button based on affordability
        if (YesButton)
        {
            YesButton->SetIsEnabled(BridgeZone->CanPlayerAffordBridge());
        }
    }
    else if (BridgeZone->BridgeState == EBridgeZoneState::BuiltCanTravel)
    {
        // Show travel prompt
        if (TitleText)
        {
            TitleText->SetText(FText::FromString("Travel Across Bridge?"));
        }

        if (DescriptionText)
        {
            DescriptionText->SetText(FText::FromString("Would you like to cross the bridge to the next island?"));
        }

        if (CostText)
        {
			// output the destination level name as the "cost"
			FString CostString = FString::Printf(TEXT("Destination: %s"), *BridgeZone->DestinationLevelName.ToString());
			CostText->SetText(FText::FromString(CostString));
            CostText->SetColorAndOpacity(FLinearColor::White);
        }

        if (YesButton)
        {
            YesButton->SetIsEnabled(true);
        }
    }
}

void UBridgePromptWidget::OnYesButtonClicked()
{
    if (!BridgeZone)
        return;

    if (BridgeZone->BridgeState == EBridgeZoneState::NeedsToBuild)
    {
        BridgeZone->OnPlayerAcceptBuild();
    }
    else if (BridgeZone->BridgeState == EBridgeZoneState::BuiltCanTravel)
    {
        BridgeZone->OnPlayerAcceptTravel();
    }
}

void UBridgePromptWidget::OnNoButtonClicked()
{
    if (BridgeZone)
    {
        BridgeZone->OnPlayerDecline();
    }
}