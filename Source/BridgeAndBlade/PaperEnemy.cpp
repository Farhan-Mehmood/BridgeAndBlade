// Fill out your copyright notice in the Description page of Project Settings.

#include "PaperEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "PaperFlipbook.h"
#include "PaperSpriteComponent.h"
#include "TimerManager.h"
#include "AIController.h"

APaperEnemy::APaperEnemy()
{
	// initialize defaults if needed (properties already have defaults above)
	LastAttackTime = -FLT_MAX;
	bIsWindingUp = false;
}

void APaperEnemy::BeginPlay()
{
	Super::BeginPlay();
}

bool APaperEnemy::CanAttack() const
{
	if (!GetWorld())
	{
		return false;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastAttackTime) >= AttackCooldown && !bIsWindingUp;
}

bool APaperEnemy::Attack(APawn* TargetPawn)
{
	if (!TargetPawn || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: invalid target or world"));
		return false;
	}

	// Check cooldown and whether already winding up
	if (!CanAttack())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Attack failed: on cooldown or already winding up"));
		return false;
	}

	// Range check
	const float DistSq = FVector::DistSquared(GetActorLocation(), TargetPawn->GetActorLocation());
	if (DistSq > (AttackRange * AttackRange))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: target out of range"));
		return false;
	}

	// Start wind-up: mark time and schedule ExecuteAttack after WindupTime.
	LastAttackTime = GetWorld()->GetTimeSeconds();
	bIsWindingUp = true;
	PendingTarget = TargetPawn;

	// Immediately stop any AI movement so the wind-up animation can play in place.
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}

	// Choose and play the appropriate wind-up flipbook (if provided), otherwise fall back to attack flipbook.
	if (GetSprite())
	{
		const FVector Delta = TargetPawn->GetActorLocation() - GetActorLocation();

		// Decide which flipbook set to use based on dominant axis (consistent with PaperBase)
		if (FMath::Abs(Delta.Y) > FMath::Abs(Delta.X))
		{
			// Side wind-up; flip X scale for left/right
			if (Delta.Y > 0)
			{
				GetSprite()->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
			}
			else
			{
				GetSprite()->SetRelativeScale3D(FVector(-1.f, 1.f, 1.f));
			}

			// Play side windup if available, otherwise play side attack as a fallback
			if (WindupSideFlipbook)
			{
				GetSprite()->SetFlipbook(WindupSideFlipbook);
				GetSprite()->PlayFromStart();
			}
			else if (AttackSideFlipbook)
			{
				GetSprite()->SetFlipbook(AttackSideFlipbook);
				GetSprite()->PlayFromStart();
			}
		}
		else
		{
			// Up / Down wind-up
			if (Delta.X > 0)
			{
				if (WindupUpFlipbook)
				{
					GetSprite()->SetFlipbook(WindupUpFlipbook);
					GetSprite()->PlayFromStart();
				}
				else if (AttackUpFlipbook)
				{
					GetSprite()->SetFlipbook(AttackUpFlipbook);
					GetSprite()->PlayFromStart();
				}
			}
			else
			{
				if (WindupDownFlipbook)
				{
					GetSprite()->SetFlipbook(WindupDownFlipbook);
					GetSprite()->PlayFromStart();
				}
				else if (AttackDownFlipbook)
				{
					GetSprite()->SetFlipbook(AttackDownFlipbook);
					GetSprite()->PlayFromStart();
				}
			}
		}
	}

	// Schedule damage application after WindupTime
	GetWorld()->GetTimerManager().SetTimer(WindupTimerHandle, this, &APaperEnemy::ExecuteAttack, WindupTime, false);

	UE_LOG(LogTemp, Log, TEXT("%s began wind-up to attack %s (windup=%f)"), *GetName(), *TargetPawn->GetName(), WindupTime);

	return true;
}

void APaperEnemy::ExecuteAttack()
{
	// wind-up finished
	bIsWindingUp = false;

	APawn* TargetPawn = PendingTarget.Get();
	PendingTarget = nullptr;

	if (!TargetPawn || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAttack aborted: invalid target or world"));
		// Optionally restore idle animation
		if (GetSprite() && IdleFlipbook)
		{
			GetSprite()->SetFlipbook(IdleFlipbook);
		}
		return;
	}

	// Range check again (optional)
	const float DistSq = FVector::DistSquared(GetActorLocation(), TargetPawn->GetActorLocation());
	if (DistSq > (AttackRange * AttackRange))
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAttack aborted: target moved out of range"));
		if (GetSprite() && IdleFlipbook)
		{
			GetSprite()->SetFlipbook(IdleFlipbook);
		}
		return;
	}

	// Play the actual attack flipbook (if available) when executing the hit
	if (GetSprite())
	{
		const FVector Delta = TargetPawn->GetActorLocation() - GetActorLocation();
		if (FMath::Abs(Delta.Y) > FMath::Abs(Delta.X))
		{
			if (AttackSideFlipbook)
			{
				GetSprite()->SetFlipbook(AttackSideFlipbook);
				GetSprite()->PlayFromStart();
			}
		}
		else
		{
			if (Delta.X > 0)
			{
				if (AttackUpFlipbook)
				{
					GetSprite()->SetFlipbook(AttackUpFlipbook);
					GetSprite()->PlayFromStart();
				}
			}
			else
			{
				if (AttackDownFlipbook)
				{
					GetSprite()->SetFlipbook(AttackDownFlipbook);
					GetSprite()->PlayFromStart();
				}
			}
		}
	}

	// Apply damage
	AController* InstigatorController = Cast<AController>(GetController());
	UGameplayStatics::ApplyDamage(TargetPawn, DamageAmount, InstigatorController, this, UDamageType::StaticClass());

	// Optionally revert to idle flipbook after attack finishes
	if (GetSprite() && IdleFlipbook)
	{
		// Let the attack flipbook play; for simplicity we immediately set idle if no attack flipbook was present.
		if (!(AttackUpFlipbook || AttackDownFlipbook || AttackSideFlipbook))
		{
			GetSprite()->SetFlipbook(IdleFlipbook);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s executed attack on %s for %f damage"), *GetName(), *TargetPawn->GetName(), DamageAmount);

	// clear timer handle in case
	GetWorld()->GetTimerManager().ClearTimer(WindupTimerHandle);
}

