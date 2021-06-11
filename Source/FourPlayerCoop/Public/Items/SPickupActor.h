// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SUsableActor.h"
#include "SPickupActor.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASPickupActor : public ASUsableActor
{
	GENERATED_BODY()

	UFUNCTION()
	void OnRep_IsActive();

protected:

	ASPickupActor();

	void BeginPlay() override;


	UPROPERTY(Transient, ReplicatedUsing = OnRep_IsActive)
	bool bIsActive;

	virtual void OnPickedUp();

	virtual void OnRespawned();

	void RespawnPickup();

public:

	virtual void OnUsed(APawn* InstigatorPawn) override;

	UPROPERTY(EditDefaultsOnly, Category = "Pickups")
	bool bAllowRespawn;

	UPROPERTY(EditDefaultsOnly, Category = "Pickups")
	float RespawnDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Pickups")
	float RespawnDelayRange;

	UPROPERTY(EditDefaultsOnly, Category = "Pickups")
	bool bStartActive;
};
