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

	UPROPERTY(Replicated)
	int32 NumKills;

	UPROPERTY(Replicated)
	int32 NumDeath;

	virtual void Reset() override;


public:

	void SetTeamNumber(int32 NewTeamNumber);

	UFUNCTION(BlueprintCallable, Category = "Teams")
	int32 GetTeamNumber() const;

	void AddKill();

	void AddDeath();

	void ScorePoints(int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetKills() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetDeath() const;

};
