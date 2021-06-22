// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:

	ASGameMode();

	virtual void PreInitializeComponents() override;

	/* Spawn Default player inventory */
	virtual void SpawnDefaultInventory(APawn* PlayerPawn);

	/**
	* Make sure Pawn properties are back to defaults
	* Also a good place to modify them on spawn 
	*/
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	/* Can we deal damage to players in the same team */
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bAllowFriendlyFireDamage;

	/* The teamnumber assigned to Players */
	int32 PlayerTeamNum;

	/* Called once on every new player that enters the gamemode */
	virtual FString InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;


public:

	/* Default Weapons to spawn with */
	UPROPERTY(EditDefaultsOnly, Category = "Player Defaults")
	TArray<TSubclassOf<class ASWeapon>> DefaultInventoryClass;

	/***************************************************************/
	/* Damage and Killing                                          */
	/***************************************************************/

	virtual void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType);

	/* Can the player deal damage according to gamemode rules (eg. friendly-fire disabled) */
	virtual bool CanDealDamage(class ASPlayerState* DamageCauser, class ASPlayerState* DamagedPlayer) const;

	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

};
