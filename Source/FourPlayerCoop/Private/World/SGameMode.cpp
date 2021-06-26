// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SGameMode.h"
#include "Player/SPlayerController.h"
#include "Player/SCharacter.h"
#include "Items/SWeapon.h"
#include "Player/SPlayerState.h"



ASGameMode::ASGameMode()
{
	// Set Default Classes
	PlayerControllerClass = ASPlayerController::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	bAllowFriendlyFireDamage = false;

	/* NOTE: Need to set bWantsPlayerState to true in AIController to assign PlayerState to the AI,
	   and to set its default team number to 0 (0 and 1 for the AI and Players resp.) */
	PlayerTeamNum = 1;
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


FString ASGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	ASPlayerState* NewPlayerState = Cast<ASPlayerState>(NewPlayerController->PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->SetTeamNumber(PlayerTeamNum);
	}

	return Result;
}


void ASGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType)
{
	// Do nothing (can we used to apply score or keep track of kill count)
}


bool ASGameMode::CanDealDamage(ASPlayerState* DamageCauser, ASPlayerState* DamagedPlayer) const
{
	if (bAllowFriendlyFireDamage)
	{
		return true;
	}

	// Allow Damage to self 
	if (DamagedPlayer == DamageCauser)
	{
		return true;
	}

	// TODO: Compare Team numbers //
	return DamageCauser && DamagedPlayer && (DamageCauser->GetTeamNumber() != DamagedPlayer->GetTeamNumber());
}


float ASGameMode::ModifyDamage(float Damage, AActor* DamagedActor, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	ASBaseCharacter* DamagedPawn = Cast<ASBaseCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		ASPlayerState* DamagedPlayerState = Cast<ASPlayerState>(DamagedPawn->GetPlayerState());
		ASPlayerState* InstigatorPlayerState = Cast<ASPlayerState>(EventInstigator->PlayerState);

		// Check for friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.f;
		}
	}

	return ActualDamage;
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

