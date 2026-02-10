// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "PaperBase.h"
#include "WeaponBase.h" 
#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "PaperChar.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UDataTable;

UCLASS()
class BRIDGEANDBLADE_API APaperChar : public APaperBase
{
    GENERATED_BODY()

public:
    APaperChar();
    
protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Database")
    UDataTable* ItemDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    FVector2D FacingDirection;

    void UpdateWeaponRotation();

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UInventoryWidget> InventoryWidgetClass;

    UPROPERTY()
    UInventoryWidget* InventoryWidget;

    bool bIsInventoryOpen;
    void OnInventoryInput();

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditAnywhere, Category = "Components")
    UCameraComponent* Camera;


    // Input
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveUpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveRightAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* ZoomCameraAction;


    // Weapon positioning
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FVector WeaponRelativeLocation;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FRotator WeaponRelativeRotation;


    // Inventory
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* EquippedWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FName> WeaponInventory;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FName, int> MaterialInventory;


    // Inventory functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItemToInventory(FName ItemName, int Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void EquipWeapon(int InventoryIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UnequipWeapon();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int GetItemCount(FName ItemName) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(FName ItemName, int Amount = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemName, int Amount = 1);

    // Crafting
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(FName ItemName) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(FName ItemName);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Attack();

    float LastAttackTime;
    bool bCanAttack;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* InventoryAction;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleInventory();

    // Movement input handlers
    void MoveUp(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void ZoomCamera(const FInputActionValue& Value);
};