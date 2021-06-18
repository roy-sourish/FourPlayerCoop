// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SWeaponInstant.h"
#include "Player/SPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "FourPlayerCoop/FourPlayerCoop.h"
#include "DrawDebugHelpers.h"

ASWeaponInstant::ASWeaponInstant()
{
	WeaponRange = 15000;
}

void ASWeaponInstant::FireWeapon()
{
	const FVector AimDir = GetAdjustedAim();
	const FVector CameraPos = GetCameraDamageStartLocation(AimDir);
	const FVector EndPos = CameraPos + (AimDir * WeaponRange);

	/* Check for impact by tracing from the camera position */
	FHitResult Impact = WeaponTrace(CameraPos, EndPos);

	const FVector MuzzleOrigin = GetMuzzleLocation();

	FVector AdjustedAimDir = AimDir;

	if (Impact.bBlockingHit)
	{
		/* Adjust the shoot direction to hit crosshair */	
		AdjustedAimDir = (Impact.ImpactPoint - MuzzleOrigin).GetSafeNormal();

		/* Retrace with the new aim direction coming out of the weapon muzzle */	
		Impact = WeaponTrace(MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange));
		
		// Debug weapon trace 
		DrawDebugLine(GetWorld(), MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange), FColor::Red, false, 2.0f, 0.0f, 1.0f);
	}
	else
	{
		/* Use the maximum distance as the adjusted direction */
		Impact.ImpactPoint = FVector_NetQuantize(EndPos);
	}

	// Process Instant Hit 

	//DrawDebugLine(GetWorld(), CameraPos, Impact.ImpactPoint, FColor::Yellow, true, 2.0f, 0.0f, 1.0f);
}
