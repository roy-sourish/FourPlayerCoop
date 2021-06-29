// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SAmmoPickup.h"
#include "Items/SWeapon.h"
#include "Player/SCharacter.h"

ASAmmoPickup::ASAmmoPickup()
{
	bAllowRespawn = true;

	AmountOfAmmo = 200;
}

void ASAmmoPickup::OnUsed(APawn* InstigatorPawn)
{
	ASCharacter* MyPawn = Cast<ASCharacter>(InstigatorPawn);
	if (MyPawn && MyPawn->IsAlive() && MyPawn->GetCurrentWeapon())
	{
		ASWeapon* MyWeapon = Cast<ASWeapon>(MyPawn->CurrentWeapon);
		if (MyWeapon)
		{
			MyWeapon->SetAmmoCount(AmountOfAmmo);
		}
		Super::OnUsed(InstigatorPawn);
	}
}
