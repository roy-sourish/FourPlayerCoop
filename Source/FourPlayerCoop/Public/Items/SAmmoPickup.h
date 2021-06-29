// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SPickupActor.h"
#include "SAmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASAmmoPickup : public ASPickupActor
{
	GENERATED_BODY()

protected:

	/* Amount of Ammo to pick up */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 AmountOfAmmo;

public:

	ASAmmoPickup();

	virtual void OnUsed(APawn* InstigatorPawn) override;
};
