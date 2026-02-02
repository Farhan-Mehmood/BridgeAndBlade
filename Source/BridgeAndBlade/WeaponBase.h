// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "WeaponBase.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Swing UMETA(DisplayName = "Swing"),
    Stab UMETA(DisplayName = "Stab"),
    AoE UMETA(DisplayName = "Area of Effect")
};

UCLASS()
class BRIDGEANDBLADE_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBase();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPaperSpriteComponent* WeaponSprite;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* AttackPoint;

public:
    virtual void Tick(float DeltaTime) override;

    // Weapon Stats
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    FName WeaponName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    float Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    EAttackType AttackType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    float AttackRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    float AttackSpeed;

    // Sprite for the weapon
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    UPaperSprite* WeaponSpriteAsset;

    // Animation properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    float SwingAngle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    float SwingDuration;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    float StabDistance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    float StabDuration;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* AttackMontage;

    // Visibility control
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void SetWeaponVisible(bool bVisible);

    // Attack Functions
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void PerformAttack(AActor* Attacker);

protected:
    virtual void SwingAttack(AActor* Attacker);
    virtual void StabAttack(AActor* Attacker);
    virtual void AoEAttack(AActor* Attacker);

    void DealDamage(AActor* Target, AActor* Attacker);

    void AnimateSwing();
    void AnimateStab();
    void ResetWeaponTransform();

    bool bIsAnimating;
    float AnimationTimer;
    FVector InitialRelativeLocation;
    FRotator InitialRelativeRotation;
    EAttackType CurrentAnimationType;
};
