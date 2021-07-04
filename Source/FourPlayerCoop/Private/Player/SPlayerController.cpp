// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SPlayerController.h"
#include "Player/SPlayerCameraManager.h"
#include "Player/SCharacter.h"
#include "Player/SBaseCharacter.h"
#include "Player/SPlayerState.h"
#include "World/SCoopGameState.h"
#include "UI/SHUD.h"

ASPlayerController::ASPlayerController(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set Default Classes to use 
	PlayerCameraManagerClass = ASPlayerCameraManager::StaticClass();

	bRespawnImmediately = false;
}


void ASPlayerController::StartSpectating()
{
	/* Update State on Server */
	PlayerState->SetIsSpectator(true);

	/* Waiting to respawn */
	bPlayerIsWaiting = true;
	ChangeState(NAME_Spectating);

	/* Push State update to the client */	
	ClientGotoState(NAME_Spectating);

	/* Focus on the remaining alive player*/
	ViewAPlayer(1);

	/* Update HUD to show the spectator Screen */
	ClientHUDStateChanged(EHUDState::Spectating);

}


void ASPlayerController::UnFreeze()
{
	Super::UnFreeze();
	
	// Check if Match is ending or has ended
	ASCoopGameState* MyGameState = GetWorld()->GetGameState<ASCoopGameState>();
	if (MyGameState && MyGameState->HasMatchEnded())
	{
		// Do not allow spectating or respawn 
		return;
	}

	// Respawn or Spectate 
	if (bRespawnImmediately)
	{
		ServerRestartPlayer();
	}
	else
	{
		StartSpectating();
	}
}


void ASPlayerController::Suicide()
{
	if (IsInState(NAME_Playing))
	{
		ServerSuicide();
	}
}


void ASPlayerController::ServerSuicide_Implementation()
{
	ASCharacter* MyPawn = Cast<ASCharacter>(GetPawn());
	if (MyPawn && ((GetWorld()->TimeSeconds - MyPawn->CreationTime > 1 ) || (GetNetMode() == NM_Standalone)))
	{
		MyPawn->Suicide();
	}
}


bool ASPlayerController::ServerSuicide_Validate()
{
	return true;
}


void ASPlayerController::ClientHUDStateChanged_Implementation(EHUDState NewState)
{
	ASHUD* HUD = Cast<ASHUD>(GetHUD());
	if (HUD)
	{
		HUD->OnStateChanged(NewState);
	}
}


void ASPlayerController::ClientHUDMessage_Implementation(EHUDMessage MessageID)
{
	/* Turn the ID into a message for the HUD to display */
	const FText TextMessage = GetText(MessageID);

	ASHUD* HUD = Cast<ASHUD>(GetHUD());
	if (HUD)
	{
		HUD->MessageReceieved(TextMessage);
	}
}


/* Temporarily set the namespace. If it ws omitted we should call NSLOCTEXT(Namespace, x, y) instead */
#define LOCTEXT_NAMESPACE "HUDMESSAGES"

FText ASPlayerController::GetText(EHUDMessage MsgID) const
{
	switch (MsgID)
	{
	case EHUDMessage::Weapon_SlotTaken:
		return LOCTEXT("WeaonSlotTaken", "Weapon Slot already taken.");
	case EHUDMessage::Health_Restored:
		return LOCTEXT("CharacterHealthRestored", "Health Restored");
	default:
		return FText::FromString("No Message Set");
	}
}

/* Remove the namespace definition so it doesnot exist in the other files compiled after this */
#undef LOCTEXT_NAMESPACE