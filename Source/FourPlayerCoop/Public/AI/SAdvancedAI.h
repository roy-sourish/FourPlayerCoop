// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/SCharacter.h"
#include "SAdvancedAI.generated.h"

/**
 * 
 */
UCLASS(ABSTRACT)
class FOURPLAYERCOOP_API ASAdvancedAI : public ASCharacter
{
	GENERATED_BODY()


public:

	ASAdvancedAI(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

protected:

	UFUNCTION()
	void OnSeePlayer(APawn* Pawn);

	UFUNCTION()
	void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

private:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UPawnSensingComponent* PawnSensingComp;

	/* Last time the player was spotted */	
	float LastSeenTime;

	/* Last time the player was heard */
	float LastHeardTime;

	/* Time-out value to clear the sensed position of the player. Should be higher than Sense interval in the PawnSense component not never miss sense ticks. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SensedTimeOut;

	/* Resets after sense time-out to avoid unnecessary clearing of target each tick */
	bool bSensedTarget;

};
