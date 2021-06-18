// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SWeapon.h"
#include "SWeaponInstant.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class FOURPLAYERCOOP_API ASWeaponInstant : public ASWeapon
{
	GENERATED_BODY()
	
protected:
	
	ASWeaponInstant();

	/*****************************************************************/
	/* Damage Processing                                             */
	/*****************************************************************/

	virtual void FireWeapon() override;



	/*****************************************************************/
	/* Weapon Configuration                                          */
	/*****************************************************************/

	UPROPERTY(EditDefaultsOnly)
	float WeaponRange;
};
