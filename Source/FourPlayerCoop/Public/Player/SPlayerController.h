// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/SHUD.h"
#include "SPlayerController.generated.h"

enum class EHUDState : uint8;

UENUM()
enum class EHUDMessage : uint8
{
	/* Weapons */	
	Weapon_SlotTaken,

	/* Character */	
	Health_Restored,

	/* No category specified */
	None
};
/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()

	ASPlayerController(const class FObjectInitializer& ObjectInitializer);

	/* Flag to start respawn or start spectating upon death */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	bool bRespawnImmediately;

	

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSuicide();
	
	void ServerSuicide_Implementation();
	
	bool ServerSuicide_Validate();

public:	

	/* Respawn or start spectating after death */
	virtual void UnFreeze() override;

	UFUNCTION(exec)
	virtual void Suicide();

	/* Start Spectating. Should be called only on server */
	void StartSpectating();

public:

	FText GetText(EHUDMessage MsgID) const;

	UFUNCTION(Reliable, Client)
	void ClientHUDStateChanged(EHUDState NewState);
	
	void ClientHUDStateChanged_Implementation(EHUDState NewState);

	UFUNCTION(Reliable, CLient)
	void ClientHUDMessage(EHUDMessage MessageID);
	
	void ClientHUDMessage_Implementation(EHUDMessage MessageID);
};
