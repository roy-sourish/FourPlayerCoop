// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SPickupActor.h"
#include "SWeaponPickup.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class FOURPLAYERCOOP_API ASWeaponPickup : public ASPickupActor
{
	GENERATED_BODY()

protected:
	
	ASWeaponPickup();

public:

	/* Weapon Class to add to inventory when picked up */
	UPROPERTY(EditDefaultsOnly, Category = "WeaponClass")
	TSubclassOf<class ASWeapon> WeaponClass;

	virtual void OnUsed(APawn* InstigatorPawn) override;
	
};
