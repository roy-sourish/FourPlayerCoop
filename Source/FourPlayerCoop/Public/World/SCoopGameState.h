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
	void WaveStateChanged(EWaveState NewState, EWaveState OldState, int32 CurrentWaveCount);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "Game State")
	EWaveState WaveState;

	UPROPERTY(Replicated)
	int32 WaveCount;

	/* Total accumulated score from all players */
	UPROPERTY(Replicated)
	int32 TotalScore;

public:

	ASCoopGameState();

	void SetWaveState(EWaveState NewState, int32 NewWaveCount);

	void AddScore(int32 Score);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetTotalScore() const;
	
};
