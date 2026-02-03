#include "PaperBase.h"

APaperBase::APaperBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bHasMoved = false; // Initialize to false
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
    // Optional: Handle idle state if you want
    // else if (!bHasMoved)
    // {
    //     GetSprite()->SetFlipbook(IdleFlipbook);
    // }
}

void APaperBase::TakeAHit(int damageAmount)
{
    health -= damageAmount;

    if (health <= 0)
    {
        die(itemDrops);
    }
}

void APaperBase::die(TArray<FString> drops)
{
    Destroy();
}