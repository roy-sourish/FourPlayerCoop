// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SGameState.h"
#include "Net/UnrealNetwork.h"

ASGameState::ASGameState()
{

}

void ASGameState::SetWaveState(EWaveState NewState)
{
	if (HasAuthority())
	{
		EWaveState OldState = WaveState;

		WaveState = NewState;

		// Call on Server
		OnRep_WaveState(OldState);
	}
}


void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, WaveState);
}
