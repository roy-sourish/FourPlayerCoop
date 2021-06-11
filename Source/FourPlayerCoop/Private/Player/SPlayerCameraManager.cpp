// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SPlayerCameraManager.h"
#include "Player/SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

ASPlayerCameraManager::ASPlayerCameraManager()
{
	NormalFOV = 90.0f;
	TargetingFOV = 65.0f;

	ViewPitchMin = -80.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;

	/* Camera Delta between standing and crouched */
	MaxCrouchOffsetZ = 46.0f;

	DefaultCameraOffsetZ = 0.0f;

	CrouchLerpVelocity = 12.0f;
}


void ASPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	ASCharacter* MyPawn = PCOwner ? Cast<ASCharacter>(PCOwner->GetPawn()) : nullptr;
	if (MyPawn)
	{
		const float TargetFOV = MyPawn->IsTargeting() ? TargetingFOV : NormalFOV;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
		SetFOV(DefaultFOV);
	}

	/* Apply smooth camera lerp between crouch togelling */	
	if (MyPawn)
	{
		if (MyPawn->bIsCrouched && !bWasCrouched)
		{
			CurrentCrouchOffset = MaxCrouchOffsetZ;
		}
		else if (!MyPawn->bIsCrouched && bWasCrouched)
		{
			CurrentCrouchOffset = -MaxCrouchOffsetZ;
		}

		bWasCrouched = MyPawn->bIsCrouched;

		/* Clamp the lerp to 0-1.0 range and interpolate to our new crouch offset */
		CurrentCrouchOffset = FMath::Lerp(CurrentCrouchOffset, 0.0f, FMath::Clamp(CrouchLerpVelocity * DeltaTime, 0.0f, 1.0f));

		FVector CurrentCameraOffset = MyPawn->GetCameraComponent()->GetRelativeTransform().GetLocation();
		FVector NewCameraOffset = FVector(CurrentCameraOffset.X, CurrentCameraOffset.Y, DefaultCameraOffsetZ + CurrentCrouchOffset);
		MyPawn->GetCameraComponent()->SetRelativeLocation(NewCameraOffset);
	}

	Super::UpdateCamera(DeltaTime);
}
