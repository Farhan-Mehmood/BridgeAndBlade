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
class UPlayerUIWidget;

UCLASS()
class BRIDGEANDBLADE_API APaperChar : public APaperBase
{
    GENERATED_BODY()

public:
    APaperChar();
    
protected:
    virtual void BeginPlay() override;

    // Unarmed combat settings
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float UnarmedDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float UnarmedAttackRange;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float UnarmedAttackSpeed;

	void PerformUnarmedAttack();

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

    

    // Player UI
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPlayerUIWidget> PlayerUIClass;

    UPROPERTY()
    UPlayerUIWidget* PlayerUIWidget;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Force refresh of the HUD quick-slot visuals
    UFUNCTION(BlueprintCallable, Category = "UI")
    void RefreshQuickSlots();

    // Quick slots (5)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "QuickSlots")
    TArray<FName> QuickSlots;

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


    // Quick slot input actions (bind these in your DefaultMappingContext)
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* QuickSlot1Action;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* QuickSlot2Action;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* QuickSlot3Action;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* QuickSlot4Action;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* QuickSlot5Action;

    // Handlers for quick slot inputs
    UFUNCTION()
    void OnQuickSlot1();

    UFUNCTION()
    void OnQuickSlot2();

    UFUNCTION()
    void OnQuickSlot3();

    UFUNCTION()
    void OnQuickSlot4();

    UFUNCTION()
    void OnQuickSlot5();


    // Weapon positioning
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FVector WeaponRelativeLocation;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FRotator WeaponRelativeRotation;


    // Inventory
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* EquippedWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    FName EquippedWeaponName;

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

    // Quick slot functions
    UFUNCTION(BlueprintCallable, Category = "QuickSlots")
    void AssignQuickSlot(int SlotIndex, FName ItemName);

    UFUNCTION(BlueprintCallable, Category = "QuickSlots")
    void UseQuickSlot(int SlotIndex);

    // Movement input handlers
    void MoveUp(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void ZoomCamera(const FInputActionValue& Value);

public:
    // Core Attributes
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attributes")
    float BaseDefense;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
    float TotalDefense;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attributes")
    float BaseAttack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
    float TotalAttack;

    // Armor Inventory Map - Maps an enum slot to an FName (the item's ID in Data Table)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Armor")
    TMap<EArmorSlot, FName> EquippedArmor;

    // Armor / Stats Functions
    UFUNCTION(BlueprintCallable, Category = "Inventory|Armor")
    void EquipArmor(FName ArmorItemName);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Armor")
    void UnequipArmor(EArmorSlot SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void RecalculateStats();

    // Combat
    virtual void TakeAHit(int32 damageAmount) override;
};