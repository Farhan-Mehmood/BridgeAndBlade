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
#include "PlayerUIWidget.h"
#include "SaveGameManager.h"

// In constructor, initialize quick slots to 5 empty entries
APaperChar::APaperChar()
{
    PrimaryActorTick.bCanEverTick = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->bUsePawnControlRotation = false;
	cameraDistance = 1216.f;

    AutoPossessPlayer = EAutoReceiveInput::Player0;

    EquippedWeapon = nullptr;
    LastAttackTime = 0.0f;
    bCanAttack = true;
    FacingDirection = FVector2D(1.f, 0.f); // Start facing right

    bIsInventoryOpen = false;
    InventoryWidget = nullptr;

    UnarmedDamage = 1.0f;
    UnarmedAttackRange = 200.0f;
    UnarmedAttackSpeed = 1.5f; // Slightly faster

	QuickSlots.Init(NAME_None, 5);

	// Initialize base stats
	BaseDefense = 0.0f;
	TotalDefense = 0.0f;
	BaseAttack = 1.0f; // Default base attack
	TotalAttack = 1.0f;
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
    AddItemToInventory(TEXT("Wood"), 15);
    AddItemToInventory(TEXT("Stone"), 10);

    CraftItem(TEXT("WoodSword"));
	CraftItem(TEXT("StoneSword"));
    EquipWeapon(0);

	AddItemToInventory(TEXT("Meat"), 5);

	// Create player HUD
	if (PC && PlayerUIClass)
	{
		PlayerUIWidget = CreateWidget<UPlayerUIWidget>(PC, PlayerUIClass);
		if (PlayerUIWidget)
		{
			PlayerUIWidget->SetOwningCharacter(this);
			PlayerUIWidget->AddToViewport();
			PlayerUIWidget->SetVisibility(ESlateVisibility::Visible);

			UE_LOG(LogTemp, Log, TEXT("PlayerUIWidget created."));

			// Initialize health display
			PlayerUIWidget->SetHealthText(health);

			// Initialize quick slots display via helper
			RefreshQuickSlots();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create PlayerUIWidget instance from PlayerUIClass"));
		}
	}
	else
	{
		if (!PC) UE_LOG(LogTemp, Warning, TEXT("PlayerController missing; cannot create HUD"));
		if (!PlayerUIClass) UE_LOG(LogTemp, Warning, TEXT("PlayerUIClass not set on APaperChar"));
	}

    // Auto-load save if it exists
    USaveGameManager* SaveManager = USaveGameManager::Get(this);
    if (SaveManager && SaveManager->DoesSaveExist(TEXT("PlayerSaveSlot")))
    {
        SaveManager->LoadGame(this, TEXT("PlayerSaveSlot"));
        UE_LOG(LogTemp, Log, TEXT("Save game auto-loaded"));
    }
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

	// Sync health to UI (tick fallback)
	if (PlayerUIWidget)
	{
		PlayerUIWidget->SetHealthText(health);
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
		EIC->BindAction(QuickSaveAction, ETriggerEvent::Started, this, &APaperChar::QuickSave);
		EIC->BindAction(QuickLoadAction, ETriggerEvent::Started, this, &APaperChar::QuickLoad);

        // Quick slot bindings (bind each action to its handler)
        if (QuickSlot1Action) EIC->BindAction(QuickSlot1Action, ETriggerEvent::Started, this, &APaperChar::OnQuickSlot1);
        if (QuickSlot2Action) EIC->BindAction(QuickSlot2Action, ETriggerEvent::Started, this, &APaperChar::OnQuickSlot2);
        if (QuickSlot3Action) EIC->BindAction(QuickSlot3Action, ETriggerEvent::Started, this, &APaperChar::OnQuickSlot3);
        if (QuickSlot4Action) EIC->BindAction(QuickSlot4Action, ETriggerEvent::Started, this, &APaperChar::OnQuickSlot4);
        if (QuickSlot5Action) EIC->BindAction(QuickSlot5Action, ETriggerEvent::Started, this, &APaperChar::OnQuickSlot5);
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

    if (Camera && FMath::Abs(AxisValue) > KINDA_SMALL_NUMBER && cameraDistance > 0 && cameraDistance < 10000)
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
           // PC->bShowMouseCursor = false;
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
    case EItemType::Placeable:
    case EItemType::Armor:        
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

        // Store the name and refresh stats
        EquippedWeaponName = WeaponName;
        RecalculateStats();

        // Refresh the UI if it's open
        if (InventoryWidget && bIsInventoryOpen)
        {
            InventoryWidget->RefreshEquipment();
        }
    }
}

void APaperChar::UnequipWeapon()
{
    if (EquippedWeapon)
    {
        EquippedWeapon->Destroy();
        EquippedWeapon = nullptr;
        EquippedWeaponName = NAME_None;

        RecalculateStats();

        // Refresh the UI if it's open
        if (InventoryWidget && bIsInventoryOpen)
        {
            InventoryWidget->RefreshEquipment();
        }
    }
}

void APaperChar::Attack()
{
    if (!bCanAttack)
    {
        return;
    }

    if (!GetWorld())
    {
        return;
    }

    // Check cooldown - use weapon speed if equipped, otherwise unarmed speed
    float AttackSpeed = EquippedWeapon ? EquippedWeapon->AttackSpeed : UnarmedAttackSpeed;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < (1.0f / AttackSpeed))
    {
        return;
    }

    // Determine a world-space target position from the mouse cursor
    APlayerController* PC = Cast<APlayerController>(GetController());
    FVector TargetLocation;
    bool bHaveTarget = false;

    if (APaperCharPlayerController* PCC = Cast<APaperCharPlayerController>(PC))
    {
        // Prefer a physics/UI hit under cursor
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

    // Save rotation, rotate to face the target (2D)
    const FRotator OldRotation = GetActorRotation();
    if (bHaveTarget)
    {
        FVector Direction = TargetLocation - GetActorLocation();
        Direction.Z = 0.0f;
        if (!Direction.IsNearlyZero())
        {
            const FRotator LookRot = Direction.Rotation();
            SetActorRotation(LookRot);
            // Update FacingDirection used by UpdateWeaponRotation (normalized)
            const FVector2D NewFacing = FVector2D(Direction.GetSafeNormal().X, Direction.GetSafeNormal().Y);
            FacingDirection = NewFacing;
        }
    }

    // Perform attack - either with weapon or unarmed
    if (EquippedWeapon)
    {
        // Weapon attack
        EquippedWeapon->PerformAttack(this);

        if (EquippedWeapon->AttackMontage)
        {
            PlayAnimMontage(EquippedWeapon->AttackMontage);
        }
    }
    else
    {
        // Unarmed attack
        PerformUnarmedAttack();
    }

    // Reset and play weapon attack flipbook
    GetSprite()->SetFlipbook(nullptr);

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
            // Flip sprite for left attack
            GetSprite()->SetRelativeScale3D(FVector(-1.f, 1.f, 1.f));
        }
    }

    LastAttackTime = CurrentTime;
    bCanAttack = false;
}

void APaperChar::PerformUnarmedAttack()
{
	UE_LOG(LogTemp, Log, TEXT("Performing unarmed attack"));

    // Simple forward punch/swing detection
    FVector StartLocation = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * UnarmedAttackRange);

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    // Line trace for unarmed attack
    if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation,
        ECC_Pawn, QueryParams))
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 0.5f);

        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            APaperBase* PaperChar = Cast<APaperBase>(HitActor);
            if (PaperChar)
            {
                PaperChar->TakeAHit(UnarmedDamage);
            }

            UE_LOG(LogTemp, Log, TEXT("Unarmed attack hit: %s for %f damage"),
                *HitActor->GetName(), UnarmedDamage);
        }
    }

	// Debug line to visualize attack range
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 0.5f);
}

// AssignQuickSlot implementation (push-to-front, remove duplicates, drop last if overflow)
void APaperChar::AssignQuickSlot(int SlotIndex, FName ItemName)
{
	if (ItemName.IsNone())
		return;

	UItemDatabase* DB = UItemDatabase::Get(this);
	if (DB)
	{
		FItemData ItemData;
		if (DB->GetItemData(ItemName, ItemData))
		{
			// Block armor being added to quick slots programmatically
			if (ItemData.ItemType == EItemType::Armor)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot assign Armor to Quick Slots."));
				return;
			}
		}
	}

	// Desired capacity (should be 5, initialized in constructor)
	const int Capacity = QuickSlots.Num() > 0 ? QuickSlots.Num() : 5;

	// Build new ordered list: item to front, then previous items excluding the item itself
	TArray<FName> NewSlots;
	NewSlots.Reserve(Capacity);

	// Insert the new item at front
	NewSlots.Add(ItemName);

	for (const FName& Existing : QuickSlots)
	{
		if (NewSlots.Num() >= Capacity)
			break;

		// Skip the item we're inserting (remove duplicates)
		if (Existing == ItemName)
			continue;

		NewSlots.Add(Existing);
	}

	// If we have fewer than capacity (e.g. previously had empty slots), fill with NAME_None
	while (NewSlots.Num() < Capacity)
	{
		NewSlots.Add(NAME_None);
	}

	QuickSlots = MoveTemp(NewSlots);

	// Log state for debugging
	for (int i = 0; i < QuickSlots.Num(); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("QuickSlot[%d] = %s"), i + 1, QuickSlots[i].IsNone() ? TEXT("None") : *QuickSlots[i].ToString());
	}

	// Update HUD
	RefreshQuickSlots();
}

// UseQuickSlot implementation (basic behavior)
void APaperChar::UseQuickSlot(int SlotIndex)
{
	if (!QuickSlots.IsValidIndex(SlotIndex))
		return;

	FName ItemName = QuickSlots[SlotIndex];
	if (ItemName.IsNone())
		return;

	UItemDatabase* DB = UItemDatabase::Get(this);
	if (!DB)
		return;

	FItemData ItemData;
	if (!DB->GetItemData(ItemName, ItemData))
		return;

	switch (ItemData.ItemType)
	{
	case EItemType::Weapon:
		{
			// Find this weapon in weapon inventory and equip first instance
			for (int i = 0; i < WeaponInventory.Num(); ++i)
			{
				if (WeaponInventory[i] == ItemName)
				{
					EquipWeapon(i);
					return;
				}
			}

			// If not present in weapon inventory, add it and equip
			AddItemToInventory(ItemName, 1);
			EquipWeapon(WeaponInventory.Num() - 1);
		}
		break;

	case EItemType::Consumable:
		{
			// Consume one and apply simple effect (example: heal 10)
			if (RemoveItem(ItemName, 1))
			{
				health += 10; // simple heal example
				if (health < 0) health = 0;
				
				// Update UI health immediately
				if (PlayerUIWidget) PlayerUIWidget->SetHealthText(health);

				// CHECK IF THIS WAS THE EXACT LAST ITEM WE HAD
				if (GetItemCount(ItemName) <= 0)
				{
					QuickSlots[SlotIndex] = NAME_None;
					RefreshQuickSlots();
				}
			}
		}
		break;

	case EItemType::Material:
	case EItemType::Placeable:
	default:
		// No default action; you can implement blueprint override or expand behavior
		UE_LOG(LogTemp, Log, TEXT("Used quick slot item: %s (no default action)"), *ItemName.ToString());
		break;
	}
}

// Quick slot input handlers
void APaperChar::OnQuickSlot1()
{
	UseQuickSlot(0);
}

void APaperChar::OnQuickSlot2()
{
	UseQuickSlot(1);
}

void APaperChar::OnQuickSlot3()
{
	UseQuickSlot(2);
}

void APaperChar::OnQuickSlot4()
{
	UseQuickSlot(3);
}

void APaperChar::OnQuickSlot5()
{
	UseQuickSlot(4);
}

// Refresh HUD quick-slot visuals
void APaperChar::RefreshQuickSlots()
{
	if (!PlayerUIWidget)
		return;

	UItemDatabase* DB = UItemDatabase::Get(this);
	for (int i = 0; i < QuickSlots.Num(); ++i)
	{
		FItemData ItemData;
		if (DB && DB->GetItemData(QuickSlots[i], ItemData))
		{
			PlayerUIWidget->SetQuickSlot(i, ItemData);
		}
		else
		{
			// Pass an empty struct to clear slot
			PlayerUIWidget->SetQuickSlot(i, FItemData());
		}
	}
}

void APaperChar::EquipArmor(FName ArmorItemName)
{
    if (!ItemDataTable) return;

    // Find the item
    FString ContextString;
    FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ArmorItemName, ContextString);

    if (ItemData && ItemData->ItemType == EItemType::Armor)
    {
        // Unequip currently equipped armor in that slot first (Refund to inventory)
        if (EquippedArmor.Contains(ItemData->ArmorSlot))
        {
            FName OldArmor = EquippedArmor[ItemData->ArmorSlot];
            AddItemToInventory(OldArmor, 1);
        }

        // Equip new armor
        EquippedArmor.Add(ItemData->ArmorSlot, ArmorItemName);

        // Remove from standard inventory 
        RemoveItem(ArmorItemName, 1);

        // Refresh stats math
        RecalculateStats();

        // Refresh the open UI!
        if (InventoryWidget && bIsInventoryOpen)
        {
            InventoryWidget->RefreshInventory(); // Call full refresh to move the item from scrollbox to equip slot
        }
    }
}

void APaperChar::UnequipArmor(EArmorSlot SlotIndex)
{
    if (EquippedArmor.Contains(SlotIndex))
    {
        FName ArmorToUnequip = EquippedArmor[SlotIndex];
        AddItemToInventory(ArmorToUnequip, 1); // Give it back to the player
        EquippedArmor.Remove(SlotIndex);
        
        // Refresh stats math
        RecalculateStats();

        // Refresh the open UI
        if (InventoryWidget && bIsInventoryOpen)
        {
            InventoryWidget->RefreshInventory();
        }
    }
}

void APaperChar::RecalculateStats()
{
    TotalDefense = BaseDefense;

    UItemDatabase* DB = UItemDatabase::Get(this);
    if (!DB) return;

    // Calculate Armor Defense
    for (const auto& ArmorPair : EquippedArmor)
    {
        FItemData ArmorData;
        if (DB->GetItemData(ArmorPair.Value, ArmorData))
        {
            TotalDefense += ArmorData.DefenseValue;
        }
    }

    // Set Attack Power strictly from the equipped weapon, or fall back to BaseAttack if unarmed
    if (EquippedWeapon)
    {
        TotalAttack = EquippedWeapon->Damage;
    }
    else
    {
        TotalAttack = BaseAttack; // Unarmed Damage
    }
    
    // Log it out to prove the math is working behind the scenes
    UE_LOG(LogTemp, Log, TEXT("Recalculated Stats - Attack: %f | Defense: %f"), TotalAttack, TotalDefense);
}

void APaperChar::TakeAHit(int damageAmount)
{
	// Calculate how much damage to block based on defense
	int MitigatedDamage = damageAmount - FMath::RoundToInt(TotalDefense);

	// Ensure the player takes at least 1 damage from attacks, so they can't be fully invincible
	MitigatedDamage = FMath::Max(1, MitigatedDamage);

	// Apply the blocked damage to health
	health -= MitigatedDamage;
	
	// Prevent health from going below zero
	if (health < 0) health = 0;

	// Update health display immediately
	if (PlayerUIWidget)
	{
		PlayerUIWidget->SetHealthText(health);
	}

	UE_LOG(LogTemp, Log, TEXT("Raw Damage: %d | Defense: %f | Took Damage: %d | Current Health: %d"), 
		damageAmount, TotalDefense, MitigatedDamage, health);
		
	
	if (health <= 0)
	{
		
		TArray<FName> emptyDrops;
		TArray<int> emptyAmounts;
		die(emptyDrops, emptyAmounts); 
	}
}

void APaperChar::SaveGame()
{
    USaveGameManager* SaveManager = USaveGameManager::Get(this);
    if (SaveManager)
    {
        bool bSuccess = SaveManager->SaveGame(this, TEXT("PlayerSaveSlot"));
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Game saved successfully"));
            // Optional: Show UI notification
        }
    }
}

void APaperChar::LoadGame()
{
    USaveGameManager* SaveManager = USaveGameManager::Get(this);
    if (SaveManager)
    {
        bool bSuccess = SaveManager->LoadGame(this, TEXT("PlayerSaveSlot"));
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Game loaded successfully"));
            // Optional: Show UI notification
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No save game found"));
        }
    }
}

void APaperChar::QuickSave()
{
    USaveGameManager* SaveManager = USaveGameManager::Get(this);
    if (SaveManager)
    {
        SaveManager->SaveGame(this, TEXT("QuickSaveSlot"));
        UE_LOG(LogTemp, Log, TEXT("Quick save completed"));
    }
}

void APaperChar::QuickLoad()
{
    USaveGameManager* SaveManager = USaveGameManager::Get(this);
    if (SaveManager)
    {
        if (SaveManager->DoesSaveExist(TEXT("QuickSaveSlot")))
        {
            SaveManager->LoadGame(this, TEXT("QuickSaveSlot"));
            UE_LOG(LogTemp, Log, TEXT("Quick load completed"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No quick save found"));
        }
    }
}