// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "PaperBase.h"

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

    // FIX: Rotate the sprite so it points Forward (X-axis) instead of UP (Z-axis)
    // Adjust Pitch (-90 or 90) depending on which way the hilt/blade needs to face
    WeaponSprite->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

    // Create attack point
    AttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttackPoint"));
    AttackPoint->SetupAttachment(WeaponSprite);

    // Default values
    Damage = 1.0f;
    AttackType = EAttackType::Swing;
    
    // Increased base attack range from 150.0f to 250.0f
    AttackRange = 250.0f;
    
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

    // Physically rotate the entire weapon actor to match the attacker
    SetActorRotation(Attacker->GetActorRotation());

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
    if (!Attacker) return;

    // Start swing animation
    bIsAnimating = true;
    AnimationTimer = 0.0f;
    CurrentAnimationType = EAttackType::Swing;

    FVector StartLocation = AttackPoint->GetComponentLocation();
    FVector ForwardVector = Attacker->GetActorForwardVector();

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Attacker);
    QueryParams.AddIgnoredActor(this);

    // Instead of tracing lines that can have gaps, we grab everything in a sphere around the attack point
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(AttackRange);

    GetWorld()->OverlapMultiByChannel(OverlapResults, StartLocation, FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);

    // Draw the full range faintly for debug
    DrawDebugSphere(GetWorld(), StartLocation, AttackRange, 16, FColor::Yellow, false, 1.0f);

    TSet<AActor*> HitActors;
    
    for (const FOverlapResult& Overlap : OverlapResults)
    {
        AActor* HitActor = Overlap.GetActor();
        if (HitActor && !HitActors.Contains(HitActor))
        {
            // Now we check if the enemy is in front of the player using Dot Product.
            // This creates a perfect solid mathematical "Cone" of attack.
            FVector DirToTarget = (HitActor->GetActorLocation() - StartLocation).GetSafeNormal2D();
            FVector Forward2D = ForwardVector.GetSafeNormal2D();

            float DotProduct = FVector::DotProduct(Forward2D, DirToTarget);

            // A DotProduct > 0.25 gives a very generous wide arc (approx ~150 degrees front-facing)
            if (DotProduct > 0.25f)
            {
                HitActors.Add(HitActor);
                DrawDebugLine(GetWorld(), StartLocation, HitActor->GetActorLocation(), FColor::Red, false, 1.0f, 0, 3.0f);
                DealDamage(HitActor, Attacker);
            }
            else
            {
                // Ignored targets (behind the player but in range)
                DrawDebugLine(GetWorld(), StartLocation, HitActor->GetActorLocation(), FColor::Blue, false, 1.0f, 0, 1.0f);
            }
        }
    }
}

void AWeaponBase::StabAttack(AActor* Attacker)
{
	UE_LOG(LogTemp, Log, TEXT("%s performs a stab attack!"), *Attacker->GetName());

    // Start stab animation
    bIsAnimating = true;
    AnimationTimer = 0.0f;
    CurrentAnimationType = EAttackType::Stab;

    FVector StartLocation = AttackPoint->GetComponentLocation();
    FVector ForwardVector = Attacker->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * AttackRange);
    

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Attacker);
    QueryParams.AddIgnoredActor(this);

    // Using a thick 45-unit sphere makes it extremely hard to accidentally miss an adjacent stab
    FCollisionShape SweepShape = FCollisionShape::MakeSphere(45.0f);

    if (GetWorld()->SweepSingleByChannel(Hit, StartLocation, EndLocation, FQuat::Identity, ECC_Pawn, SweepShape, QueryParams))
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1.0f, 0, 3.0f);
        DealDamage(Hit.GetActor(), Attacker);
    }
    else
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
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

    APaperBase* PaperChar = Cast<APaperBase>(Target);
    if (PaperChar)
    {
		PaperChar->TakeAHit(Damage);
		UE_LOG(LogTemp, Log, TEXT("%s dealt %.1f damage to %s"), *Attacker->GetName(), Damage, *Target->GetName());
	}
}

void AWeaponBase::AnimateSwing()
{
    AActor* WeaponOwner = GetOwner();
    if (!WeaponOwner) return;

    // Calculate swing progress (0 to 1)
    float Progress = FMath::Clamp(AnimationTimer / SwingDuration, 0.0f, 1.0f);
    float SwingProgress = FMath::Sin(Progress * PI);

    // Swing the yaw using the Owner's world rotation as a base!
    FRotator BaseRotation = WeaponOwner->GetActorRotation();
    BaseRotation.Yaw += SwingAngle * SwingProgress;

    // Rotate the entire Actor
    SetActorRotation(BaseRotation);
}

void AWeaponBase::AnimateStab()
{
    AActor* WeaponOwner = GetOwner();
    if (!WeaponOwner) return;

    // Calculate stab progress (0 to 1)
    float Progress = FMath::Clamp(AnimationTimer / StabDuration, 0.0f, 1.0f);
    float StabProgress = FMath::Sin(Progress * PI);

    // EXACTLY what the attack hitbox uses - move along the owner's true Forward Vector
    FVector ForwardVector = WeaponOwner->GetActorForwardVector();
    
    // Instead of local space, push the actor forward in actual 3D space
    FVector NewLocation = WeaponOwner->GetActorLocation() + (ForwardVector * (StabDistance * StabProgress));
    
    SetActorLocation(NewLocation);
}

void AWeaponBase::ResetWeaponTransform()
{
    AActor* WeaponOwner = GetOwner();

    // Snap back to the default position on the character
    if (WeaponOwner)
    {
        SetActorLocation(WeaponOwner->GetActorLocation());
        SetActorRelativeRotation(FRotator::ZeroRotator);
    }
    
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