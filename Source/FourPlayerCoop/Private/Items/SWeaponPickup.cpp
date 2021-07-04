// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SWeaponPickup.h"
#include "Items/SWeapon.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerController.h"

ASWeaponPickup::ASWeaponPickup()
{
	// Set Respawn to false 
	bAllowRespawn = false;

	/* Enable support simulated physics movement when weapons are dropped by a player */
	SetReplicateMovement(true);
}

void ASWeaponPickup::OnUsed(APawn* InstigatorPawn)
{
	ASCharacter* MyPawn = Cast<ASCharacter>(InstigatorPawn);
	if (MyPawn)
	{	
		/* if storage slot available, spawn and add to inventory */
		if (MyPawn->WeaponSlotAvailable(WeaponClass->GetDefaultObject<ASWeapon>()->GetStorageSlot()))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, SpawnParams);

			// Add to Inventory
			MyPawn->AddWeapon(NewWeapon);

			// Call base class (SPickupActor) to hide mesh and disable collision then Destroy Actor(if bAllowRespawn = false) 
			Super::OnUsed(InstigatorPawn);
		}
		else
		{
			// @TODO: Display HUD Error Message 
			
			ASPlayerController* PC = Cast<ASPlayerController>(MyPawn->GetController());
			if (PC)
			{
				PC->ClientHUDMessage(EHUDMessage::Weapon_SlotTaken);
			}
		}
	}
}
