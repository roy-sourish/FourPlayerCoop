// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SCharacter.h"
#include "Components/SCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/SUsableActor.h"
#include "Items/SWeapon.h"
#include "FourPlayerCoop/STypes.h"


ASCharacter::ASCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->GravityScale = 1.5;
	MoveComp->JumpZVelocity = 620;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->MaxWalkSpeedCrouched = 200;

	// Enable Crouching 
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraBoomComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoomComp->bUsePawnControlRotation = true;
	CameraBoomComp->SetupAttachment(GetRootComponent());

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(CameraBoomComp);

	MaxUseDistance = 500.0f;

	// Weapon Sockets 
	WeaponAttachPoint = TEXT("WeaponSocket");
	SecondaryAttachPoint = TEXT("SecondarySocket");
	SpineAttachPoint = TEXT("SpineSocket");
}


void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWantsToRun && !IsSprinting())
	{
		SetSprinting(true);
	}

	if (Controller && Controller->IsLocalController())
	{
		ASUsableActor* Usable = GetUsableInView();

		/* End Focus */
		if (FocusedUsableActor != Usable)
		{
			bHasNewFocus = true;

			if (FocusedUsableActor)
			{
				FocusedUsableActor->OnEndFocus();
			}
		}

		/* Set New Focus Item */
		FocusedUsableActor = Usable;

		// Start Focus 
		if (Usable)
		{
			if (bHasNewFocus)
			{
				Usable->OnBeginFocus();
				bHasNewFocus = false;
			}
		}

	}
}


void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("SprintHold", IE_Pressed, this, &ASCharacter::OnStartSprinting);
	PlayerInputComponent->BindAction("SprintHold", IE_Released, this, &ASCharacter::OnStopSprinting);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::OnJump);

	PlayerInputComponent->BindAction("CrouchToggle", IE_Released, this, &ASCharacter::OnCrouchToggle);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &ASCharacter::OnStartTargeting);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &ASCharacter::OnStopTargeting);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &ASCharacter::Use);

	PlayerInputComponent->BindAction("PrimaryWeapon", IE_Pressed, this, &ASCharacter::OnEquipPrimaryWeapon);
	PlayerInputComponent->BindAction("SecondaryWeapon", IE_Pressed, this, &ASCharacter::OnEquipSecondaryWeapon);
}


void ASCharacter::MoveForward(float Val)
{
	if (Controller && Val != 0.0f)
	{
		// Limit Pitch when walking or falling 
		const bool bLimitRotation = (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->IsMovingOnGround());
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);

		AddMovementInput(Direction, Val);
	}
}


void ASCharacter::MoveRight(float Val)
{
	if (Val != 0.0f)
	{
		const FRotator Rotation = GetActorRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

		AddMovementInput(Direction, Val);
	}
}


void ASCharacter::OnCrouchToggle()
{
	if (CanCrouch())
	{
		Crouch();	
	}
	else
	{
		UnCrouch();
	}
}


void ASCharacter::OnJump()
{
	SetIsJumping(true);
}


void ASCharacter::OnStartSprinting()
{
	SetSprinting(true);
}


void ASCharacter::OnStopSprinting()
{
	SetSprinting(false);
}


void ASCharacter::SetSprinting(bool NewSprinting)
{
	Super::SetSprinting(NewSprinting);
}


bool ASCharacter::IsInitiatedJump() const
{
	return bIsJumping;
}


void ASCharacter::SetIsJumping(bool NewJumping)
{
	if (bIsCrouched && NewJumping)
	{
		UnCrouch();
	}
	else if (NewJumping != bIsJumping)
	{
		bIsJumping = NewJumping;
		
		if (bIsJumping)
		{
			Jump();
		}
	}

	if (!HasAuthority())
	{
		ServerSetIsJumping(NewJumping);
	}
}


void ASCharacter::ServerSetIsJumping_Implementation(bool NewJumping)
{
	SetIsJumping(NewJumping);
}


bool ASCharacter::ServerSetIsJumping_Validate(bool NewJumping)
{
	return true;
}


void ASCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	/* Check if we are no longer falling/Jumping */
	if (PrevMovementMode == EMovementMode::MOVE_Falling &&
		GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
	{
		SetIsJumping(false);
	}
}


void ASCharacter::OnStartTargeting()
{
	SetTargeting(true);
}


void ASCharacter::OnStopTargeting()
{
	SetTargeting(false);
}


void ASCharacter::Use()
{
	if (HasAuthority())
	{
		ASUsableActor* Usable = GetUsableInView();
		if (Usable)
		{
			Usable->OnUsed(this);
		}
	}
	else
	{
		ServerUse();
	}
}


void ASCharacter::ServerUse_Implementation()
{
	Use();
}


bool ASCharacter::ServerUse_Validate()
{
	return true;
}


ASUsableActor* ASCharacter::GetUsableInView() const
{
	FVector CamLoc;
	FRotator CamRot;

	if (Controller == nullptr)
		return nullptr;

	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector TraceStart = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector TraceEnd = TraceStart + (Direction * MaxUseDistance);

	FCollisionQueryParams QueryParams(TEXT("TraceParams"), true, this);
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.bTraceComplex = true;

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// return focused actor 
	return (Cast<ASUsableActor>(Hit.GetActor()));
}


void ASCharacter::OnEquipPrimaryWeapon()
{
	if (Inventory.Num() >= 1)
	{
		/* Find first weapon that uses primary slot and equip it */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			ASWeapon* Weapon = Inventory[i];
			if (Weapon && Weapon->GetStorageSlot() == EInventorySlot::Primary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}


void ASCharacter::OnEquipSecondaryWeapon()
{
	if (Inventory.Num() >= 2)
	{
		/* Find second weapon that uses secondary slot */
		for (int i = 0; i < Inventory.Num(); i++)
		{
			ASWeapon* Weapon = Inventory[i];
			if (Weapon && Weapon->GetStorageSlot() == EInventorySlot::Secondary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}


void ASCharacter::EquipWeapon(ASWeapon* Weapon)
{
	if (Weapon)
	{
		/* Ignore if trying to equip already equipped weapon */	
		if (Weapon == CurrentWeapon)
		{
			return;
		}

		/* Push to server if client */
		if (HasAuthority())
		{
			// Current weapon will be the prev. weapon when switching to new weapon
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}


void ASCharacter::ServerEquipWeapon_Implementation(ASWeapon* Weapon)
{
	EquipWeapon(Weapon);
}


bool ASCharacter::ServerEquipWeapon_Validate(ASWeapon* Weapon)
{
	return true;
}


void ASCharacter::OnRep_CurrentWeapon(ASWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}


ASWeapon* ASCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}


FName ASCharacter::GetInventoryAttachPoints(EInventorySlot Slot) const
{
	switch (Slot)
	{
	case EInventorySlot::Hands:
		return WeaponAttachPoint;
	case EInventorySlot::Primary:
		return SpineAttachPoint;
	case EInventorySlot::Secondary:
		return SecondaryAttachPoint;
	default:
		return "";
	}
}


void ASCharacter::AddWeapon(ASWeapon* Weapon)
{
	if (Weapon && HasAuthority())
	{
		// Set Owning Pawn and Attach the weapons to player mesh 
		Weapon->OnEnterInventory(this);

		// Add default weapons to inventory
		Inventory.AddUnique(Weapon);

		// Equip first weapon in inventory 
		if (Inventory.Num() > 0 && CurrentWeapon == nullptr)
		{
			EquipWeapon(Inventory[0]);
		}
	}
}


void ASCharacter::SetCurrentWeapon(ASWeapon* NewWeapon, ASWeapon* LastWeapon)
{

	/* Maintain visual reference to previous weapon - used in SwapToNewWeaponMesh*/	
	PreviousWeapon = LastWeapon;

	ASWeapon* LocalLastWeapon = nullptr;
	if (LastWeapon)		// LastWeapon = Current weapon that the player is holding 
	{
		LocalLastWeapon = LastWeapon;
		UE_LOG(LogTemp, Error, TEXT("if(LastWeapon)"));
	}
	else if (NewWeapon != CurrentWeapon)
	{
		// When Player Hands is empty the val of Current Weapon is nullptr, and LocalLastWepon is set to nullptr.
	
		LocalLastWeapon = CurrentWeapon;	
		UE_LOG(LogTemp, Error, TEXT("if(NewWeapon != CurrentWeapon)"));
	}

	// UnEquip the current weapon 
	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
		bHasPreviousWeapon = true;
	}

	CurrentWeapon = NewWeapon;

	// Equip New Weapon
	if (NewWeapon)
	{
		// Set Owner 
		NewWeapon->SetOwningPawn(this);

		// Only play animation when we are holding an item in hand 
		NewWeapon->OnEquip(bHasPreviousWeapon);
	}

	/* NOTE: If you don't have an equip animation w/ animnotify to swap the meshes halfway through,
			 then uncomment this to immediately swap instead */
	//SwapToNewWeaponMesh();
}


void ASCharacter::SwapToNewWeaponMesh()
{
	if (PreviousWeapon)
	{
		PreviousWeapon->AttachMeshToPawn(PreviousWeapon->GetStorageSlot());
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachMeshToPawn(EInventorySlot::Hands);
	}
}


void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASCharacter, bIsJumping, COND_SkipOwner);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, Inventory);
}