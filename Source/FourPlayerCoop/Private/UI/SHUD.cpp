// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SHUD.h"

ASHUD::ASHUD()
{

}

EHUDState ASHUD::GetCurrentState() const
{
	return CurrentState;
}

void ASHUD::OnStateChanged_Implementation(EHUDState NewState)
{
	CurrentState = NewState;
}
