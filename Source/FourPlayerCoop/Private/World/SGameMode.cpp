// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SGameMode.h"
#include "Player/SPlayerController.h"
#include "Player/SCharacter.h"
#include "Items/SWeapon.h"



ASGameMode::ASGameMode()
{
	// Set Default Classes
	PlayerControllerClass = ASPlayerController::StaticClass();
}


void ASGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}


void ASGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	SpawnDefaultInventory(PlayerPawn);
}


void ASGameMode::SpawnDefaultInventory(APawn* PlayerPawn)
{
	ASCharacter* MyPawn = Cast<ASCharacter>(PlayerPawn);
	if (MyPawn)
	{
		for (int32 i = 0; i < DefaultInventoryClass.Num(); i++)
		{
			if (DefaultInventoryClass[i])
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultInventoryClass[i], SpawnInfo);

				// Adds weapon to inventory and attaches it to player socket
				MyPawn->AddWeapon(NewWeapon);
			}
		}
	}
}

