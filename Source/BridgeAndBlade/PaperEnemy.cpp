// Fill out your copyright notice in the Description page of Project Settings.

#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "PaperFlipbook.h"
#include "PaperSpriteComponent.h"

APaperEnemy::APaperEnemy()
{
	// initialize defaults if needed (properties already have defaults above)
	LastAttackTime = -FLT_MAX;
}

void APaperEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Example: Set Actor Location next to player
	//SetActorLocation(FVector(1190.f, 700.f, 3490.f));
}

bool APaperEnemy::CanAttack() const
{
	if (!GetWorld())
	{
		return false;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastAttackTime) >= AttackCooldown;
}

bool APaperEnemy::Attack(APawn* TargetPawn)
{
	if (!TargetPawn || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: invalid target or world"));
		return false;
	}

	// Check cooldown first
	if (!CanAttack())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Attack failed: on cooldown"));
		return false;
	}

	// Range check
	const float DistSq = FVector::DistSquared(GetActorLocation(), TargetPawn->GetActorLocation());
	if (DistSq > (AttackRange * AttackRange))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: target out of range"));
		return false;
	}

	// Mark the attack time (cooldown)
	LastAttackTime = GetWorld()->GetTimeSeconds();

	// Apply damage to the target. You can replace this with any custom damage/attack logic.
	AController* InstigatorController = Cast<AController>(GetController());
	UGameplayStatics::ApplyDamage(TargetPawn, DamageAmount, InstigatorController, this, UDamageType::StaticClass());

	// Trigger attack animation from PaperBase flipbooks if available.
	// Decide which attack flipbook to play based on relative position to target.
	//if (GetSprite())
	//{
	//	const FVector Delta = TargetPawn->GetActorLocation() - GetActorLocation();
	//	UPaperFlipbook* Chosen = nullptr;

	//	// Use X axis as "up/down" and Y as "side" for flipbook selection.
	//	if (FMath::Abs(Delta.X) >= FMath::Abs(Delta.Y))
	//	{
	//		// Vertical preference
	//		if (Delta.X >= 0.f)
	//		{
	//			Chosen = AttackUpFlipbook ? AttackUpFlipbook : IdleFlipbook;
	//		}
	//		else
	//		{
	//			Chosen = AttackDownFlipbook ? AttackDownFlipbook : IdleFlipbook;
	//		}
	//	}
	//	else
	//	{
	//		// Side attack
	//		Chosen = AttackSideFlipbook ? AttackSideFlipbook : IdleFlipbook;
	//	}

	//	if (Chosen)
	//	{
	//		GetSprite()->SetFlipbook(Chosen);
	//		// If you want to ensure the animation restarts from the beginning:
	//		GetSprite()->PlayFromStart();
	//	}
	//}

	// TODO: fire attack sound, VFX, animation notifies, and any hit timing logic.
	UE_LOG(LogTemp, Log, TEXT("%s attacked %s for %f damage"), *GetName(), *TargetPawn->GetName(), DamageAmount);

	return true;
}

