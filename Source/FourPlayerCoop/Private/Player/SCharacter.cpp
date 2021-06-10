// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SCharacter.h"
#include "Components/SCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"


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
}


void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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


void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASCharacter, bIsJumping, COND_SkipOwner);
}