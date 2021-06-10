// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API USCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual float GetMaxSpeed() const override;
};
