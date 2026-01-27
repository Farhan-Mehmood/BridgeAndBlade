// Fill out your copyright notice in the Description page of Project Settings.


#include "PaperBase.h"

APaperBase::APaperBase()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    //PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    //PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerMesh"));
    //PlayerMesh->SetupAttachment(RootComponent);
}

void APaperBase::BeginPlay()
{
}

void APaperBase::Tick(float DeltaTime)
{
}

void APaperBase::UpdateAnimation()
{
    const float Speed = GetVelocity().Size();
    const float MoveY = GetVelocity().Y;
    const float MoveX = GetVelocity().X;

    if (Speed > 5.f && !bIsMoving)
    {
        if (MoveX < MoveY)
        {
            if (MoveY > 0)
            {
                GetSprite()->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
                GetSprite()->SetFlipbook(WalkSideFlipbook);
                bIsMoving = true;
            }
            else
            {
                GetSprite()->SetRelativeScale3D(FVector(-1.f, -1.f, 1.f));
                GetSprite()->SetFlipbook(WalkSideFlipbook);
                bIsMoving = true;
            }
        }
        else
        {
            if (MoveX > 0)
            {
                GetSprite()->SetFlipbook(WalkUpFlipbook);
                bIsMoving = true;
            }
            else
            {
                GetSprite()->SetFlipbook(WalkDownFlipbook);
                bIsMoving = true;
            }
        }
    }
    else if (Speed <= 5.f && bIsMoving)
    {
        GetSprite()->SetFlipbook(IdleFlipbook);
        bIsMoving = false;
    }
}