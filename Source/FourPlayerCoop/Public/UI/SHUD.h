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

	/* Main HUD update loop */	
	//virtual void DrawHUD() override;

	//void DrawCenterDot();
	
};
