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

	lastHP = health; // Initialize lastHP to current health at start
}

void APaperBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAnimation();

    if (health != lastHP)
    {
        // Health has changed since last tick, so we know we took damage (or were healed)
        int DamageTaken = lastHP - health;
        FString DamageText = FString::Printf(TEXT("%d"), FMath::Abs(DamageTaken));

        // Choose color based on damage or healing
        const FColor TextColor = (DamageTaken > 0) ? FColor::Red : FColor::Green;

        // Spawn the floating text at the enemy's location, slightly above it
        FVector TextLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);

        // Draw debug string in world
        DrawDebugString(
            GetWorld(),
            TextLocation,
            DamageText,
            nullptr,
            TextColor,
            2.0f, // Duration in seconds
            true,  // Draw shadow
            2.0f   // Text scale
        );

        // Update lastHP for next comparison
        lastHP = health;

        UE_LOG(LogTemp, Log, TEXT("%s took %d damage"), *GetName(), DamageTaken);
    }
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

float APaperBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Convert to integer damage and forward to existing TakeAHit (keeps existing death/drops logic)
    const int32 DamageInt = FMath::RoundToInt(DamageAmount);
    if (DamageInt != 0)
    {
        TakeAHit(DamageInt);
    }

    // Return actual damage applied (match engine convention)
    return DamageAmount;
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