// PaperBase.cpp
#include "PaperBase.h"
#include "PaperChar.h"
#include "Kismet/GameplayStatics.h"

APaperBase::APaperBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bHasMoved = false;
    health = 100; // Default health
}

void APaperBase::BeginPlay()
{
    Super::BeginPlay();
}

void APaperBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAnimation();
}

void APaperBase::UpdateAnimation()
{
    const float Speed = GetVelocity().Size();
    const float MoveY = GetVelocity().Y;
    const float MoveX = GetVelocity().X;

    // Check if moving
    if (Speed > 5.f)
    {
        bHasMoved = true;
        
        // Determine which direction is dominant
        if (FMath::Abs(MoveY) > FMath::Abs(MoveX))
        {
            // Moving more horizontally (Y axis: side to side)
            if (MoveY > 0)
            {
                // Moving right
                GetSprite()->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
                GetSprite()->SetFlipbook(WalkSideFlipbook);
            }
            else
            {
                // Moving left
                GetSprite()->SetRelativeScale3D(FVector(-1.f, 1.f, 1.f));
                GetSprite()->SetFlipbook(WalkSideFlipbook);
            }
        }
        else
        {
            // Moving more vertically (X axis: up and down)
            if (MoveX > 0)
            {
                // Moving up
                GetSprite()->SetFlipbook(WalkUpFlipbook);
            }
            else
            {
                // Moving down
                GetSprite()->SetFlipbook(WalkDownFlipbook);
            }
        }
    }
}

void APaperBase::TakeAHit(int32 damageAmount)
{
    health -= damageAmount;
    
    UE_LOG(LogTemp, Warning, TEXT("%s took %d damage. Health: %d"), *GetName(), damageAmount, health);
    
    if (health <= 0)
    {
        die(itemDrops, itemDropAmounts);
    }
}

void APaperBase::die(TArray<FName> drops, TArray<int32> amounts)
{
    UE_LOG(LogTemp, Warning, TEXT("%s died"), *GetName());

    // Failsafe: if drops array is empty, just destroy
    if (drops.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("%s has no drops"), *GetName());
        Destroy();
        return;
    }

    // Find the player character
    APaperChar* PlayerChar = Cast<APaperChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    if (PlayerChar)
    {
        // Add each item to player inventory
        for (int32 i = 0; i < drops.Num(); ++i)
        {
            FName ItemName = drops[i];
            int32 Amount = 1; // Default amount
            
            // If amounts array has a corresponding entry, use it
            if (amounts.IsValidIndex(i))
            {
                Amount = amounts[i];
            }
            
            // Add item to player inventory
            PlayerChar->AddItemToInventory(ItemName, Amount);
            
            UE_LOG(LogTemp, Log, TEXT("%s dropped: %s x%d"), *GetName(), *ItemName.ToString(), Amount);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find player to give drops to"));
    }

    // Destroy the actor
    Destroy();
}