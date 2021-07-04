// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SHUD.generated.h"

UENUM(BlueprintType)
enum class EHUDState : uint8
{
	Playing,
	Spectating,
	MatchEnd
};

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASHUD : public AHUD
{
	GENERATED_BODY()

	ASHUD();

	/* Current HUD State */	
	EHUDState CurrentState;

public:

	UFUNCTION(BlueprintCallable, Category = "HUD")
	EHUDState GetCurrentState() const;

	/* An event to hook HUD text events to display in the HUD. BP HUD class must implement how to deal with this event.  */	
	UFUNCTION(BlueprintImplementableEvent, Category = "HUDEvents")
	void MessageReceieved(const FText& TextMessage);

	/* Event Hook to update HUD state (e.g. to determine visibility of widget )*/
	UFUNCTION(BlueprintNativeEvent, Category = "HUDEvent")
	void OnStateChanged(EHUDState NewState);

	// If no blueprint override of OnStateChanged() is found this will be called 
	void OnStateChanged_Implementation(EHUDState NewState);
};
