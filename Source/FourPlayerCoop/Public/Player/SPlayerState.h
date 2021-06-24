// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()

	ASPlayerState();

	/* Team number assigned to player */
	UPROPERTY(VisibleDefaultsOnly,Transient, Replicated)
	int32 TeamNumber;

public:

	void SetTeamNumber(int32 NewTeamNumber);

	UFUNCTION(BlueprintCallable, Category = "Teams")
	int32 GetTeamNumber() const;

};
