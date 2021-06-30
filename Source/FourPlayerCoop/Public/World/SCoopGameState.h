// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SCoopGameState.generated.h"


UENUM(BlueprintType)
enum class EWaveState : uint8
{
	// Default State
	WaitingToStart,

	WaveInProgress,

	// No longer spawning bots, waiting for players to kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver
};

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASCoopGameState : public AGameState
{
	GENERATED_BODY()

protected:

	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "Game State")
	EWaveState WaveState;

public:

	ASCoopGameState();

	void SetWaveState(EWaveState NewState);
	
};
