// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAdvancedAI.h"
#include "AI/SAdvancedAIController.h"
#include "Player/SBaseCharacter.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerState.h"
#include "Items/SWeapon.h"
#include "DrawDebugHelpers.h"

/* AI includes */
#include "Perception/PawnSensingComponent.h"



ASAdvancedAI::ASAdvancedAI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->SetPeripheralVisionAngle(60.0f);
	PawnSensingComp->SightRadius = 2000;
	PawnSensingComp->HearingThreshold = 600;
	PawnSensingComp->LOSHearingThreshold = 1200;

	Health = 100;
	SprintSpeedModifier = 3.0f;

}


void ASAdvancedAI::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &ASAdvancedAI::OnSeePlayer);
		PawnSensingComp->OnHearNoise.AddDynamic(this, &ASAdvancedAI::OnHearNoise);
	}

	ASPlayerState* PS = Cast<ASPlayerState>(GetPlayerState());
	if (PS)
	{
		PS->SetPlayerName("AIBot");
		PS->SetIsABot(true);
	}
}


void ASAdvancedAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* Check if last time we sensed the player is beyond the time out value to prevent bot from endlessly following a player */
	if (bSensedTarget && (GetWorld()->TimeSeconds - LastSeenTime) > SensedTimeOut
		&& (GetWorld()->TimeSeconds - LastHeardTime) > SensedTimeOut)
	{
		ASAdvancedAIController* AIController = Cast<ASAdvancedAIController>(GetController());
		if (AIController)
		{
			bSensedTarget = false;

			/* reset */
			AIController->SetTargetEnemy(nullptr);
	
			SetTargeting(false);
		}
	}
}


void ASAdvancedAI::FaceRotation(FRotator NewRotation, float DeltaTime)
{
	FRotator CurrentRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 8.0f);

	Super::FaceRotation(CurrentRotation, DeltaTime);
}


void ASAdvancedAI::OnSeePlayer(APawn* Pawn)
{
	if (!IsAlive())
	{
		return;
	}

	LastSeenTime = GetWorld()->TimeSeconds;
	bSensedTarget = true;

	ASAdvancedAIController* AIController = Cast<ASAdvancedAIController>(GetController());
	ASBaseCharacter* SensedPawn = Cast<ASBaseCharacter>(Pawn);
	if (AIController && SensedPawn)
	{
		AIController->SetTargetEnemy(SensedPawn);

		float DistToEnemy = (SensedPawn->GetActorLocation() - GetActorLocation()).Size();
		if (DistToEnemy <= 1350.0f)
		{
			SetTargeting(true);
		}
		else
		{
			SetTargeting(false);
		}
	}
}


void ASAdvancedAI::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	if (!IsAlive())
	{
		return;
	}

	LastSeenTime = GetWorld()->TimeSeconds;
	bSensedTarget = true;

	ASAdvancedAIController* AIController = Cast<ASAdvancedAIController>(GetController());
	if (AIController)
	{
		SetTargeting(true);
		AIController->SetLastKnownPosition(NoiseInstigator->GetActorLocation());
	}
}