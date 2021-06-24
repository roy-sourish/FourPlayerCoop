// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SPlayerState.h"
#include "Net/UnrealNetwork.h"

ASPlayerState::ASPlayerState()
{
	/* AI will remain in team 0, players are updated to team 1 through the GameMode::InitNewPlayer
	   NOTE: Need to set bWantsPlayerState to true in AIController to assign PlayerState to the AI,
	   and to set its default team number to 0 (0 and 1 for the AI and Players resp.) */
	TeamNumber = 0;
}

void ASPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}

int32 ASPlayerState::GetTeamNumber() const
{
	return TeamNumber;
}


void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerState, TeamNumber);
}
