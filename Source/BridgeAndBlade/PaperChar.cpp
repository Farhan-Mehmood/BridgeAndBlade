// Fill out your copyright notice in the Description page of Project Settings.


#include "PaperChar.h"
#include "ItemDatabase.h"
#include "ItemData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h" // for UEngineTypes::ConvertToTraceType
#include "PaperCharPlayerController.h"
#include "Engine/EngineTypes.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.h"

APaperChar::APaperChar()
{
    PrimaryActorTick.bCanEverTick = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->bUsePawnControlRotation = false;

    AutoPossessPlayer = EAutoReceiveInput::Player0;

    EquippedWeapon = nullptr;
    LastAttackTime = 0.0f;
    bCanAttack = true;
    FacingDirection = FVector2D(1.f, 0.f); // Start facing right

    bIsInventoryOpen = false;
    InventoryWidget = nullptr;
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

    // Initialize the item database
    if (ItemDataTable)
    {
        UItemDatabase::Get(this)->Initialize(ItemDataTable);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ItemDataTable not assigned in PaperChar Blueprint!"));
    }

    // Test crafting
    AddItemToInventory(TEXT("Wood"), 10);
    AddItemToInventory(TEXT("Stone"), 10);

    CraftItem(TEXT("WoodSword"));
	CraftItem(TEXT("StoneSword"));
    EquipWeapon(1);
}

void APaperChar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateWeaponRotation();

	FacingDirection = FVector2D(GetVelocity().GetSafeNormal2D());

	// Handle attack cooldown
    if (!bCanAttack)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastAttackTime >= 1.0f) // 1 second cooldown
        {
            bCanAttack = true;
        }
	}
}

void APaperChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &APaperChar::MoveUp);
        EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &APaperChar::MoveRight);
        EIC->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this, &APaperChar::ZoomCamera);
        EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &APaperChar::Attack);
        EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &APaperChar::OnInventoryInput);
    }
}

void APaperChar::MoveUp(const FInputActionValue& Value)
{
    if (bCanAttack)
    {
        float InputValue = Value.Get<float>();
        AddMovementInput(GetActorForwardVector() * InputValue);
    }
}

void APaperChar::MoveRight(const FInputActionValue& Value)
{
    if (bCanAttack)
    {
        float InputValue = Value.Get<float>();
        AddMovementInput(GetActorRightVector() * InputValue);
    }
}

void APaperChar::ZoomCamera(const FInputActionValue& Value)
{
    const float AxisValue = Value.Get<float>();
    const float ZoomSpeed = 50.0f;

    if (Camera && FMath::Abs(AxisValue) > KINDA_SMALL_NUMBER)
    {
        Camera->AddLocalOffset(FVector(AxisValue * ZoomSpeed, 0.f, 0.f));
    }
}

void APaperChar::OnInventoryInput()
{
    ToggleInventory();
}

void APaperChar::ToggleInventory()
{
    if (bIsInventoryOpen)
    {
        // Close inventory
        if (InventoryWidget)
        {
            InventoryWidget->RemoveFromParent();
            InventoryWidget = nullptr;
        }

        bIsInventoryOpen = false;

        // Re-enable player input
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
    }
    else
    {
        // Open inventory
        if (InventoryWidgetClass)
        {
            APlayerController* PC = Cast<APlayerController>(GetController());
            if (!PC)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get PlayerController"));
                return;
            }

            InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass); // Create with PC, not World
            if (InventoryWidget)
            {
                InventoryWidget->SetOwningCharacter(this);
                InventoryWidget->AddToViewport();

                // FIXED: Set input mode without SetWidgetToFocus
                FInputModeGameAndUI InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;

                bIsInventoryOpen = true;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create InventoryWidget"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InventoryWidgetClass is not set!"));
        }
    }
}

void APaperChar::UpdateWeaponRotation()
{
    if (!EquippedWeapon)
        return;

    float Angle = 0.f;

    // Cardinal directions
    if (FMath::Abs(FacingDirection.X) > FMath::Abs(FacingDirection.Y))
    {
        Angle = (FacingDirection.X > 0.f) ? 90.f : 270.f;  // Right : Left
    }
    else
    {
        Angle = (FacingDirection.Y > 0.f) ? 0.f : 180.f;  // Forward : Back
    }

    FRotator NewRotation = FRotator(0.f, 0.f, Angle);
    EquippedWeapon->SetActorRelativeRotation(NewRotation);
}

void APaperChar::AddItemToInventory(FName ItemName, int Amount)
{
    UItemDatabase* DB = UItemDatabase::Get(this);
    FItemData ItemData;

    if (!DB->GetItemData(ItemName, ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trying to add unknown item: %s"), *ItemName.ToString());
        return;
    }

    // Handle based on item type
    switch (ItemData.ItemType)
    {
    case EItemType::Material:
    case EItemType::Consumable:
        // Stackable items go into material inventory
        if (MaterialInventory.Contains(ItemName))
        {
            MaterialInventory[ItemName] = FMath::Min(
                MaterialInventory[ItemName] + Amount,
                ItemData.MaxStackSize
            );
        }
        else
        {
            MaterialInventory.Add(ItemName, FMath::Min(Amount, ItemData.MaxStackSize));
        }
        break;

    case EItemType::Weapon:
        // Weapons go into weapon inventory
        for (int i = 0; i < Amount; ++i)
        {
            WeaponInventory.Add(ItemName);
        }
        break;

    case EItemType::Placeable:
        // Placeables could go into a separate inventory
        // For now, treat like materials
        if (MaterialInventory.Contains(ItemName))
        {
            MaterialInventory[ItemName] += Amount;
        }
        else
        {
            MaterialInventory.Add(ItemName, Amount);
        }
        break;
    }
}

int APaperChar::GetItemCount(FName ItemName) const
{
    if (MaterialInventory.Contains(ItemName))
    {
        return MaterialInventory[ItemName];
    }

    // Count weapons
    int WeaponCount = 0;
    for (const FName& WeaponName : WeaponInventory)
    {
        if (WeaponName == ItemName)
        {
            WeaponCount++;
        }
    }

    return WeaponCount;
}

bool APaperChar::HasItem(FName ItemName, int Amount) const
{
    return GetItemCount(ItemName) >= Amount;
}

bool APaperChar::RemoveItem(FName ItemName, int Amount)
{
    if (!HasItem(ItemName, Amount))
    {
        return false;
    }

    if (MaterialInventory.Contains(ItemName))
    {
        MaterialInventory[ItemName] -= Amount;
        if (MaterialInventory[ItemName] <= 0)
        {
            MaterialInventory.Remove(ItemName);
        }
        return true;
    }

    // Remove from weapon inventory
    int RemovedCount = 0;
    for (int i = WeaponInventory.Num() - 1; i >= 0 && RemovedCount < Amount; --i)
    {
        if (WeaponInventory[i] == ItemName)
        {
            WeaponInventory.RemoveAt(i);
            RemovedCount++;
        }
    }

    return RemovedCount == Amount;
}

bool APaperChar::CanCraftItem(FName ItemName) const
{
    return UItemDatabase::Get(const_cast<APaperChar*>(this))->CanCraftItem(ItemName, MaterialInventory);
}

bool APaperChar::CraftItem(FName ItemName)
{
    UItemDatabase* DB = UItemDatabase::Get(this);
    FItemData ItemData;

    if (!DB->GetItemData(ItemName, ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Item '%s' not found in database"), *ItemName.ToString());
        return false;
    }

    if (!ItemData.bIsCraftable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item '%s' is not craftable"), *ItemName.ToString());
        return false;
    }

    if (!CanCraftItem(ItemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough materials to craft '%s'"), *ItemName.ToString());
        return false;
    }

    // Consume materials
    for (const FCraftingRequirement& Requirement : ItemData.CraftingRequirements)
    {
        RemoveItem(Requirement.ItemName, Requirement.Amount);
    }

    // Add crafted item to inventory
    AddItemToInventory(ItemName, 1);

    return true;
}

void APaperChar::EquipWeapon(int InventoryIndex)
{
    if (!WeaponInventory.IsValidIndex(InventoryIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid weapon inventory index: %d"), InventoryIndex);
        return;
    }

    UnequipWeapon();

    // Get weapon data from database
    FName WeaponName = WeaponInventory[InventoryIndex];
    UItemDatabase* DB = UItemDatabase::Get(this);
    FItemData ItemData;

    if (!DB->GetItemData(WeaponName, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("Weapon '%s' not found in database"), *WeaponName.ToString());
        return;
    }

    if (ItemData.ItemType != EItemType::Weapon || !ItemData.WeaponClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Item '%s' is not a valid weapon"), *WeaponName.ToString());
        return;
    }

    // Spawn weapon
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(
        ItemData.WeaponClass,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (EquippedWeapon)
    {
        EquippedWeapon->AttachToComponent(
            GetSprite(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            NAME_None
        );
        EquippedWeapon->SetActorRelativeLocation(WeaponRelativeLocation);
        EquippedWeapon->SetActorRelativeRotation(WeaponRelativeRotation);

    }
}

void APaperChar::UnequipWeapon()
{
    if (EquippedWeapon)
    {
        EquippedWeapon->Destroy();
        EquippedWeapon = nullptr;
    }
}

void APaperChar::Attack()
{
    if (!EquippedWeapon || !bCanAttack)
    {
        return;
    }

    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < (1.0f / EquippedWeapon->AttackSpeed))
    {
        return;
    }

    // Determine a world-space target position from the mouse cursor.
    APlayerController* PC = Cast<APlayerController>(GetController());
    FVector TargetLocation;
    bool bHaveTarget = false;
    if (APaperCharPlayerController* PCC = Cast<APaperCharPlayerController>(PC))
    {
        // prefer a physics/UI hit under cursor
        FHitResult Hit;
        if (PCC->GetHitUnderCursorByChannel(ECC_Visibility, Hit))
        {
            TargetLocation = Hit.Location;
            bHaveTarget = true;
        }
        else if (PCC->GetMouseWorldPointAtPlane(GetActorLocation().Z, TargetLocation))
        {
            bHaveTarget = true;
        }
    }

    // Save rotation, rotate to face the target (2D), perform attack, then restore rotation.
    const FRotator OldRotation = GetActorRotation();
    if (bHaveTarget)
    {
        FVector Direction = TargetLocation - GetActorLocation();
        Direction.Z = 0.0f;
        if (!Direction.IsNearlyZero())
        {
            const FRotator LookRot = Direction.Rotation();
            SetActorRotation(LookRot);

            // Update FacingDirection used by UpdateWeaponRotation (normalized).
            const FVector2D NewFacing = FVector2D(Direction.GetSafeNormal().X, Direction.GetSafeNormal().Y);
            FacingDirection = NewFacing;
        }
    }

    // Perform the weapon attack (WeaponBase uses Attacker rotation to determine sweep direction).
    EquippedWeapon->PerformAttack(this);
    LastAttackTime = CurrentTime;

	bCanAttack = false;

    if (EquippedWeapon->AttackMontage)
    {
        PlayAnimMontage(EquippedWeapon->AttackMontage);
    }

    // reset flipbook
	GetSprite()->SetFlipbook(nullptr);

	// play flipbook based on facing direction
   if (FacingDirection.Y > 0.f)
    {
        GetSprite()->SetFlipbook(AttackUpFlipbook);
    }
    else if (FacingDirection.Y < 0.f)
    {
        GetSprite()->SetFlipbook(AttackDownFlipbook);
    }
    else
    {
       GetSprite()->SetFlipbook(AttackSideFlipbook);
       if (FacingDirection.X < 0.f)
       {
		   // flip sprite for left attack
		   GetSprite()->SetRelativeScale3D(FVector(-1.f, 1.f, 1.f));
       }
   }
}