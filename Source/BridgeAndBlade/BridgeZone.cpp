// Fill out your copyright notice in the Description page of Project Settings.

#include "BridgeZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PaperChar.h"
#include "BridgePromptWidget.h"
#include "Kismet/GameplayStatics.h"

ABridgeZone::ABridgeZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // Create trigger volume
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(RootComponent);
    TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));

    // Create bridge mesh (hidden by default)
    BridgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BridgeMesh"));
    BridgeMesh->SetupAttachment(RootComponent);
    BridgeMesh->SetVisibility(false);
    BridgeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BridgeMesh->SetRelativeLocation(FVector(5000.f, 0.f, -50.f));

    // Default state
    BridgeState = EBridgeZoneState::NeedsToBuild;
    bPlayerInZone = false;
    PlayerInZone = nullptr;
    //CurrentPromptWidget = nullptr;
    DestinationLevelName = TEXT("Island_02");
}

void ABridgeZone::BeginPlay()
{
    Super::BeginPlay();

    // Bind trigger events
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ABridgeZone::OnTriggerBeginOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ABridgeZone::OnTriggerEndOverlap);

    // Set bridge visibility based on state
    if (BridgeState == EBridgeZoneState::BuiltCanTravel)
    {
        BridgeMesh->SetVisibility(true);
        BridgeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void ABridgeZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABridgeZone::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APaperChar* Player = Cast<APaperChar>(OtherActor);
    if (Player)
    {
        PlayerInZone = Player;
        bPlayerInZone = true;
        ShowPrompt();

        UE_LOG(LogTemp, Log, TEXT("Player entered bridge zone"));
    }
}

void ABridgeZone::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APaperChar* Player = Cast<APaperChar>(OtherActor);
    if (Player && Player == PlayerInZone)
    {
        bPlayerInZone = false;
        PlayerInZone = nullptr;
        HidePrompt();

        UE_LOG(LogTemp, Log, TEXT("Player left bridge zone"));
    }
}

void ABridgeZone::ShowPrompt()
{
    if (!PromptWidgetClass || !PlayerInZone)
        return;

    APlayerController* PC = Cast<APlayerController>(PlayerInZone->GetController());
    if (!PC)
        return;

    // Create widget if it doesn't exist
    if (!CurrentPromptWidget)
    {
        CurrentPromptWidget = CreateWidget<UBridgePromptWidget>(PC, PromptWidgetClass);
    }

    if (CurrentPromptWidget)
    {
        CurrentPromptWidget->SetBridgeZone(this);
        CurrentPromptWidget->UpdatePrompt();
        CurrentPromptWidget->AddToViewport(10); // High Z-order

        // Show mouse cursor
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
    }
}

void ABridgeZone::HidePrompt()
{
    if (CurrentPromptWidget)
    {
        CurrentPromptWidget->RemoveFromParent();
        CurrentPromptWidget = nullptr;
    }

    if (PlayerInZone)
    {
        APlayerController* PC = Cast<APlayerController>(PlayerInZone->GetController());
        if (PC)
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
}

bool ABridgeZone::CanPlayerAffordBridge() const
{
    if (!PlayerInZone)
        return false;

    for (const FCraftingRequirement& Cost : BuildingCost)
    {
        if (!PlayerInZone->HasItem(Cost.ItemName, Cost.Amount))
        {
            return false;
        }
    }

    return true;
}

void ABridgeZone::OnPlayerAcceptBuild()
{
    UE_LOG(LogTemp, Log, TEXT("Player accepted bridge building"));

    if (!CanPlayerAffordBridge())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player cannot afford bridge!"));
        return;
    }

    // Remove materials from player inventory
    for (const FCraftingRequirement& Cost : BuildingCost)
    {
        PlayerInZone->RemoveItem(Cost.ItemName, Cost.Amount);
    }

    // Build the bridge
    BuildBridge();

    // Hide the prompt
    HidePrompt();
	ShowPrompt(); // Show travel prompt
}

void ABridgeZone::OnPlayerAcceptTravel()
{
    UE_LOG(LogTemp, Log, TEXT("Player accepted travel"));

    HidePrompt();
    TravelToNextIsland();
}

void ABridgeZone::OnPlayerDecline()
{
    UE_LOG(LogTemp, Log, TEXT("Player declined"));
    HidePrompt();
}

void ABridgeZone::BuildBridge()
{
    BridgeState = EBridgeZoneState::BuiltCanTravel;

    // Show bridge mesh
    BridgeMesh->SetVisibility(true);
    BridgeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UE_LOG(LogTemp, Log, TEXT("Bridge built!"));
}

void ABridgeZone::TravelToNextIsland()
{
    if (DestinationLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Destination level name not set!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Traveling to level: %s"), *DestinationLevelName.ToString());

    // Load the next level
    UGameplayStatics::OpenLevel(this, DestinationLevelName);
}