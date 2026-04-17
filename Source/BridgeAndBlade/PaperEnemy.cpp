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

void APaperEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (CooldownRemaining > 0.0f)
	{
		CooldownRemaining -= DeltaTime;

		GetSprite()->SetFlipbook(CurrentFlipbook);

		UE_LOG(LogTemp, Log, TEXT("%s cooldown ticking down: %f seconds remaining"), *GetName(), CooldownRemaining);
	}
	else
	{
		CurrentFlipbook = GetSprite() ? GetSprite()->GetFlipbook() : nullptr;
	}
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
				CurrentFlipbook = AttackSideFlipbook;
				CooldownRemaining = AttackCooldown;
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
					CurrentFlipbook = AttackUpFlipbook;
					CooldownRemaining = AttackCooldown;
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
					CurrentFlipbook = AttackDownFlipbook;
					CooldownRemaining = AttackCooldown;
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

	// Play the actual attack flipbook (if available) when executing the hit
	// We want to play this animation even if the hit will miss
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

	// Range check again just to decide IF we apply damage, BUT don't return entirely
	const float DistSq = FVector::DistSquared(GetActorLocation(), TargetPawn->GetActorLocation());
	if (DistSq > (AttackRange * AttackRange))
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAttack missed: target moved out of range"));
	}
	else
	{
		// Apply damage since they are still in range
		AController* InstigatorController = Cast<AController>(GetController());
		UGameplayStatics::ApplyDamage(TargetPawn, DamageAmount, InstigatorController, this, UDamageType::StaticClass());
		UE_LOG(LogTemp, Log, TEXT("%s executed attack on %s for %f damage"), *GetName(), *TargetPawn->GetName(), DamageAmount);
	}

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

