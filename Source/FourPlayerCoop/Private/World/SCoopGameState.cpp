// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SCoopGameState.h"
#include "Net/UnrealNetwork.h"

ASCoopGameState::ASCoopGameState()
{
	WaveCount = 0;
}

void ASCoopGameState::SetWaveState(EWaveState NewState, int32 NewWaveCount)
{
	if (HasAuthority())
	{
		EWaveState OldState = WaveState;
		WaveCount = NewWaveCount;
		WaveState = NewState;

		// Call on Server
		OnRep_WaveState(OldState);
	}
}


void ASCoopGameState::AddScore(int32 Score)
{
	TotalScore += Score;
}


int32 ASCoopGameState::GetTotalScore() const
{
	return TotalScore;
}


void ASCoopGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState, WaveCount);
}

void ASCoopGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCoopGameState, WaveState);
	DOREPLIFETIME(ASCoopGameState, WaveCount);
	DOREPLIFETIME(ASCoopGameState, TotalScore);
}