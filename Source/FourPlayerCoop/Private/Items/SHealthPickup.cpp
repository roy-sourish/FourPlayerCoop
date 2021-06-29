// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SHealthPickup.h"
#include "Player/SBaseCharacter.h"
#include "Player/SCharacter.h"

ASHealthPickup::ASHealthPickup()
{
	bAllowRespawn = true;

	HealAmount = 100.0f;
}

void ASHealthPickup::OnUsed(APawn* InstigatorPawn)
{
	ASCharacter* MyPawn = Cast<ASCharacter>(InstigatorPawn);
	if (MyPawn)
	{
		if (!MyPawn->IsAlive())
		{
			return;
		}
		else
		{
			// Add Health
			MyPawn->Heal(HealAmount);

			Super::OnUsed(InstigatorPawn);
		}
	}
}
