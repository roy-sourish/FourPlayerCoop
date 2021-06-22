// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "SDamageType.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API USDamageType : public UDamageType
{
	GENERATED_BODY()

	USDamageType();

	/* Can player die from this damage type */
	bool bCanDieFrom;

	/* Damage Modifier for headshot damage */
	UPROPERTY(EditDefaultsOnly, Category = "Damage Properties")
	float HeadshotDmgModifier;

	/* Damage Modifier for limb damage */
	UPROPERTY(EditDefaultsOnly, Category = "Damage Properties")
	float LimbDmgModifier;

public:

	bool GetCanDieFrom() const;

	float GetHeadshotDamageModifier() const;

	float GetLimbDamageModifier() const;
};
