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

    if (health <= 0)
    {
        die(itemDrops);
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
        if (FMath::Abs(MoveX) > FMath::Abs(MoveY))
        {
            // Moving more horizontally
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
            // Moving more vertically
            if (MoveX > 0)
            {
                // Moving forward/up
                GetSprite()->SetFlipbook(WalkUpFlipbook);
            }
            else
            {
                // Moving backward/down
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

void APaperBase::die(TArray<FString> drops)
{
    Destroy();
}