// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BridgeZone.generated.h"

class UStaticMeshComponent;
class APaperChar;
class UBridgePromptWidget;

UENUM(BlueprintType)
enum class EBridgeZoneState : uint8
{
    NeedsToBuild UMETA(DisplayName = "Needs to Build"),
    BuiltCanTravel UMETA(DisplayName = "Built - Can Travel")
};

UCLASS()
class BRIDGEANDBLADE_API ABridgeZone : public AActor
{
    GENERATED_BODY()

public:
    ABridgeZone();

protected:
    virtual void BeginPlay() override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BridgeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootScene;

public:
    virtual void Tick(float DeltaTime) override;

    // Bridge State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge Settings")
    EBridgeZoneState BridgeState;

    // Building Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge Settings")
    TArray<FCraftingRequirement> BuildingCost;

    // Level to load when traveling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge Settings")
    FName DestinationLevelName;

    // UI Widget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UBridgePromptWidget> PromptWidgetClass;

protected:
    //UPROPERTY()
    UBridgePromptWidget* CurrentPromptWidget;

    UPROPERTY()
    APaperChar* PlayerInZone;

    bool bPlayerInZone;

    // Trigger callbacks
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // Called by the UI widget
    UFUNCTION(BlueprintCallable, Category = "Bridge")
    void OnPlayerAcceptBuild();

    UFUNCTION(BlueprintCallable, Category = "Bridge")
    void OnPlayerAcceptTravel();

    UFUNCTION(BlueprintCallable, Category = "Bridge")
    void OnPlayerDecline();

    UFUNCTION(BlueprintCallable, Category = "Bridge")
    bool CanPlayerAffordBridge() const;

protected:
    void ShowPrompt();
    void HidePrompt();
    void BuildBridge();
    void TravelToNextIsland();
};