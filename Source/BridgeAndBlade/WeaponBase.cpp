// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create weapon sprite component (instead of static mesh)
    WeaponSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("WeaponSprite"));
    WeaponSprite->SetupAttachment(RootComponent);
    WeaponSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create attack point
    AttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttackPoint"));
    AttackPoint->SetupAttachment(WeaponSprite);

    // Default values
    Damage = 1.0f;
    AttackType = EAttackType::Swing;
    AttackRange = 150.0f;
    AttackSpeed = 1.0f;
    WeaponName = TEXT("Base Weapon");

    // Animation defaults
    SwingAngle = 90.0f;
    SwingDuration = 0.3f;
    StabDistance = 50.0f;
    StabDuration = 0.2f;

    bIsAnimating = false;
    AnimationTimer = 0.0f;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponSpriteAsset)
    {
        WeaponSprite->SetSprite(WeaponSpriteAsset);
    }

    // Start invisible
    SetWeaponVisible(false);

    InitialRelativeLocation = WeaponSprite->GetRelativeLocation();
    InitialRelativeRotation = WeaponSprite->GetRelativeRotation();
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bIsAnimating)
    {
        AnimationTimer += DeltaTime;

        if (CurrentAnimationType == EAttackType::Swing)
        {
            if (AnimationTimer >= SwingDuration)
            {
                ResetWeaponTransform();
            }
            else
            {
                AnimateSwing();
            }
        }
        else if (CurrentAnimationType == EAttackType::Stab)
        {
            if (AnimationTimer >= StabDuration)
            {
                ResetWeaponTransform();
            }
            else
            {
                AnimateStab();
            }
        }
    }
}

void AWeaponBase::PerformAttack(AActor* Attacker)
{
    if (!Attacker) return;

    // Make weapon visible during attack
    SetWeaponVisible(true);

    switch (AttackType)
    {
    case EAttackType::Swing:
        SwingAttack(Attacker);
        break;
    case EAttackType::Stab:
        StabAttack(Attacker);
        break;
    case EAttackType::AoE:
        AoEAttack(Attacker);
        break;
    }
}

void AWeaponBase::SwingAttack(AActor* Attacker)
{
    // Start swing animation
    bIsAnimating = true;
    AnimationTimer = 0.0f;
    CurrentAnimationType = EAttackType::Swing;

    // Perform a sweeping arc attack
    FVector StartLocation = AttackPoint->GetComponentLocation();
    FVector ForwardVector = Attacker->GetActorForwardVector();

    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Attacker);
    QueryParams.AddIgnoredActor(this);

    // Sweep in an arc (simplified - sweep from left to right)
    for (int32 i = -45; i <= 45; i += 15)
    {
        FRotator Rotation = ForwardVector.Rotation();
        Rotation.Yaw += i;
        FVector SweepDirection = Rotation.Vector();
        FVector EndLocation = StartLocation + (SweepDirection * AttackRange);

        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation,
            ECC_Pawn, QueryParams))
        {
            HitResults.Add(Hit);
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f);
        }
    }

    // Deal damage to all hit actors
    TSet<AActor*> HitActors;
    for (const FHitResult& Hit : HitResults)
    {
        if (Hit.GetActor() && !HitActors.Contains(Hit.GetActor()))
        {
            HitActors.Add(Hit.GetActor());
            DealDamage(Hit.GetActor(), Attacker);
        }
    }
}

void AWeaponBase::StabAttack(AActor* Attacker)
{
    // Start stab animation
    bIsAnimating = true;
    AnimationTimer = 0.0f;
    CurrentAnimationType = EAttackType::Stab;

    // Perform a straight forward stab
    FVector StartLocation = AttackPoint->GetComponentLocation();
    FVector ForwardVector = Attacker->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * AttackRange);

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Attacker);
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation,
        ECC_Pawn, QueryParams))
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1.0f);
        DealDamage(Hit.GetActor(), Attacker);
    }
}

void AWeaponBase::AoEAttack(AActor* Attacker)
{
    // Perform an area of effect attack around the attacker
    FVector AttackerLocation = Attacker->GetActorLocation();

    TArray<FHitResult> HitResults;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(AttackRange);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Attacker);
    QueryParams.AddIgnoredActor(this);

    GetWorld()->SweepMultiByChannel(HitResults, AttackerLocation, AttackerLocation,
        FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);

    DrawDebugSphere(GetWorld(), AttackerLocation, AttackRange, 12, FColor::Orange, false, 1.0f);

    // Deal damage to all actors in range
    for (const FHitResult& Hit : HitResults)
    {
        if (Hit.GetActor())
        {
            DealDamage(Hit.GetActor(), Attacker);
        }
    }
}

void AWeaponBase::DealDamage(AActor* Target, AActor* Attacker)
{
    if (!Target) return;

    UGameplayStatics::ApplyDamage(Target, Damage, Attacker->GetInstigatorController(),
        Attacker, UDamageType::StaticClass());
}

void AWeaponBase::AnimateSwing()
{
    // Calculate swing progress (0 to 1)
    float Progress = FMath::Clamp(AnimationTimer / SwingDuration, 0.0f, 1.0f);

    // Use a sine wave for smooth swing motion (goes from -1 to 1 and back)
    float SwingProgress = FMath::Sin(Progress * PI);

    // Apply rotation
    FRotator NewRotation = InitialRelativeRotation;
    NewRotation.Roll += SwingAngle * SwingProgress;
    WeaponSprite->SetRelativeRotation(NewRotation);
}

void AWeaponBase::AnimateStab()
{
    // Calculate stab progress (0 to 1)
    float Progress = FMath::Clamp(AnimationTimer / StabDuration, 0.0f, 1.0f);

    // Use a sine wave for smooth stab motion (goes from 0 to 1 and back)
    float StabProgress = FMath::Sin(Progress * PI);

    // Apply forward movement (assuming weapon faces right in sprite space)
    FVector NewLocation = InitialRelativeLocation;
    NewLocation.X += StabDistance * StabProgress;
    WeaponSprite->SetRelativeLocation(NewLocation);
}

void AWeaponBase::ResetWeaponTransform()
{
    WeaponSprite->SetRelativeLocation(InitialRelativeLocation);
    WeaponSprite->SetRelativeRotation(InitialRelativeRotation);
    bIsAnimating = false;
    AnimationTimer = 0.0f;

    // Hide weapon after attack finishes
    SetWeaponVisible(false);
}

void AWeaponBase::SetWeaponVisible(bool bVisible)
{
    if (WeaponSprite)
    {
        WeaponSprite->SetVisibility(bVisible);
    }
}