// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SCharacterMovementComponent.h"
#include "Player/SBaseCharacter.h"

float USCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const ASBaseCharacter* CharOwner = Cast<ASBaseCharacter>(PawnOwner);

	if (CharOwner)
	{
		if (CharOwner->IsTargeting() && !IsCrouching())
		{
			MaxSpeed *= CharOwner->GetTargetingSpeedModifier();
		}
		else if (CharOwner->IsSprinting())
		{
			MaxSpeed *= CharOwner->GetSprintSpeedModifier();
		}
	}
	
	return MaxSpeed;
}
