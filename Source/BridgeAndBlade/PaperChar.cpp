// Fill out your copyright notice in the Description page of Project Settings.


#include "PaperChar.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
APaperChar::APaperChar()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    //PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerMesh"));
    //PlayerMesh->SetupAttachment(RootComponent);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(PlayerMesh);
    Camera->bUsePawnControlRotation = true;

    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    AutoPossessPlayer = EAutoReceiveInput::Player0;

}

void APaperChar::BeginPlay()
{
	Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

        if (Subsystem)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void APaperChar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const FVector Velocity = GetVelocity();
    const float MoveX = Velocity.X;

    // Deadzone to avoid jitter
    if (FMath::Abs(MoveX) > 5.f)
    {
        const float FacingScale = (MoveX > 0.f) ? 1.f : -1.f;
        GetSprite()->SetRelativeScale3D(FVector(FacingScale, 1.f, 1.f));
    }

	UpdateAnimation();
}

void APaperChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &APaperChar::MoveUp);
        EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &APaperChar::MoveRight);
        EIC->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this, &APaperChar::ZoomCamera);
    }
}

void APaperChar::UpdateAnimation()
{
    const float Speed = GetVelocity().Size();
	const float MoveY = GetVelocity().Y;
	const float MoveX = GetVelocity().X;

    if (Speed > 5.f && !bIsRunning)
    {
        if (MoveX < MoveY)
        {
            if (MoveY > 0)
            {
                GetSprite()->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
                GetSprite()->SetFlipbook(RunSideFlipbook);
                bIsRunning = true;
            }
            else
            {
				GetSprite()->SetRelativeScale3D(FVector(-1.f, -1.f, 1.f));
                GetSprite()->SetFlipbook(RunSideFlipbook);
                bIsRunning = true;
			}
        }
        else
        {
            if (MoveX > 0)
            {
                GetSprite()->SetFlipbook(RunUpFlipbook);
                bIsRunning = true;
            }
           else
            {
                GetSprite()->SetFlipbook(RunDownFlipbook);
                bIsRunning = true;
			}
        }
    }
    else if (Speed <= 5.f && bIsRunning)
    {
        GetSprite()->SetFlipbook(IdleFlipbook);
        bIsRunning = false;
    }
}

// Movement input handlers
void APaperChar::MoveUp(const FInputActionValue& Value)
{
    AddMovementInput(GetActorForwardVector() * Value.Get<float>());
}

void APaperChar::MoveRight(const FInputActionValue& Value)
{
    AddMovementInput(GetActorRightVector() * Value.Get<float>());
}

void APaperChar::ZoomCamera(const FInputActionValue& Value)
{
    // Simple zoom by moving camera along its local X axis. Tune ZoomSpeed as needed.
    const float AxisValue = Value.Get<float>();
    const float ZoomSpeed = 20.0f;

    if (Camera && FMath::Abs(AxisValue) > KINDA_SMALL_NUMBER)
    {
        Camera->AddLocalOffset(FVector(AxisValue * ZoomSpeed, 0.f, 0.f));
    }
}
