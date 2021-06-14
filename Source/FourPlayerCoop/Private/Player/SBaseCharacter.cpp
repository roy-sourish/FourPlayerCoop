// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SBaseCharacter.h"
#include "Components/SCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASBaseCharacter::ASBaseCharacter(const class FObjectInitializer& ObjectInitializer)
	/* Override the movement class from the base class to our own to support multiple speeds (eg. sprinting) */
	:Super(ObjectInitializer.SetDefaultSubobjectClass<USCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	SprintSpeedModifier = 2.5f;
	TargetingSpeedModifier = 0.5f;

	/* Don't collide with camera checks to keep 3rd Person Camera at position when bots or other players are standing behind us */
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}


bool ASBaseCharacter::IsSprinting() const
{
	return bWantsToRun && !IsTargeting() && !GetVelocity().IsZero()
		// Don't allow sprinting while strafing or standing still.
		// (1 is forward and -1 is backward, while near 0 sideways or standing still)
		// Changing the value to 0.1 will allow diagonal sprinting.
		&& (FVector::DotProduct(GetVelocity().GetSafeNormal2D(), GetActorRotation().Vector()) > 0.8f);
}


void ASBaseCharacter::SetSprinting(bool NewSprinting)
{
	bWantsToRun = NewSprinting;

	if (bIsCrouched)
	{
		UnCrouch();
	}

	if (!HasAuthority())
	{
		ServerSetSprinting(NewSprinting);
	}
}


float ASBaseCharacter::GetSprintSpeedModifier() const
{
	return SprintSpeedModifier;
}


void ASBaseCharacter::ServerSetSprinting_Implementation(bool NewSprinting)
{
	SetSprinting(NewSprinting);
}


bool ASBaseCharacter::ServerSetSprinting_Validate(bool NewSprinting)
{
	return true;
}


void ASBaseCharacter::SetTargeting(bool NewTargeting)
{
	bIsTargeting = NewTargeting;

	if (!HasAuthority())
	{
		ServerSetTargeting(NewTargeting);
	}
}


void ASBaseCharacter::ServerSetTargeting_Implementation(bool NewTargeting)
{
	SetTargeting(NewTargeting);
}


bool ASBaseCharacter::ServerSetTargeting_Validate(bool NewTargeting)
{
	return true;
}


bool ASBaseCharacter::IsTargeting() const
{
	return bIsTargeting;
}


float ASBaseCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}


FRotator ASBaseCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLs = AimDirLS.Rotation();
	
	return AimRotLs;
}


void ASBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASBaseCharacter, bWantsToRun, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASBaseCharacter, bIsTargeting, COND_SkipOwner);
	
}