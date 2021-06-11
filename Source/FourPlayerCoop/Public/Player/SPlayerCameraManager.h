// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
private:

	ASPlayerCameraManager();

	/* Update FOV */
	virtual void UpdateCamera(float DeltaTime) override;

	/* Hip Fire FOV */
	float NormalFOV;

	/* Targeting FOV */
	float TargetingFOV;
	
	float CurrentCrouchOffset;

	/* Max Camera offset applied when crouch is initiated. Always lerp back to zero */
	float MaxCrouchOffsetZ;

	float CrouchLerpVelocity;

	bool bWasCrouched;

	/* Default relative Z offset of player camera */
	float DefaultCameraOffsetZ;

};
