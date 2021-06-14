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

public:

	/* Default Weapons to spawn with */
	UPROPERTY(EditDefaultsOnly, Category = "Player Defaults")
	TArray<TSubclassOf<class ASWeapon>> DefaultInventoryClass;
};
