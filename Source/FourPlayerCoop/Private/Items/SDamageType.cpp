// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SDamageType.h"

USDamageType::USDamageType()
{
	HeadshotDmgModifier = 2.0f;
	LimbDmgModifier = 0.5f;

	bCanDieFrom = true;
}


bool USDamageType::GetCanDieFrom() const
{
	return bCanDieFrom;
}


float USDamageType::GetHeadshotDamageModifier() const
{
	return HeadshotDmgModifier;
}


float USDamageType::GetLimbDamageModifier() const
{
	return LimbDmgModifier;
}
