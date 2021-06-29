// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SPickupActor.h"
#include "SHealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASHealthPickup : public ASPickupActor
{
	GENERATED_BODY()
	
private:

	/* Amount of health to add */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float HealAmount;

public:

	ASHealthPickup();

	virtual void OnUsed(APawn* InstigatorPawn) override;
};
