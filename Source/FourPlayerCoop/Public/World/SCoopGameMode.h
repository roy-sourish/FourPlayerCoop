// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/SGameMode.h"
#include "SCoopGameMode.generated.h"

enum class EWaveState : uint8;


UCLASS()
class FOURPLAYERCOOP_API ASCoopGameMode : public ASGameMode
{
	GENERATED_BODY()

protected:

	FTimerHandle TimerHandle_BotSpawnner;

	FTimerHandle TimerHandle_NextWaveStart;

	int32 NrOfBotsToSpawn;

	int32 WaveCount;

	/* Time between each wave */
	UPROPERTY(EditDefaultsOnly, Category = "Wave Spawn Properties")
	float TimeBetweenWaves;


	/*===================================================================================*/
	/*===================================================================================*/


	/* Hook for BP to spawn a single bot */
	UFUNCTION(BlueprintImplementableEvent, Category = "CoopGameMode")
	void SpawnNewBot();

	/* Keeps track of number of spawnned bots in current wave */
	void SpawnBotTimerElapsed();

	/* Start a new wave */
	void StartWave();

	/* End current wave */
	void EndWave();

	/* Prepare for next wave */
	void PrepareForNextWave();

	/* Start next wave only when previous wave is complete and no bots are remaining */
	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

public:

	ASCoopGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

};
