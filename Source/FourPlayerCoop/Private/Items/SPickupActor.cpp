// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SPickupActor.h"
#include "Net/UnrealNetwork.h"

ASPickupActor::ASPickupActor()
{
	/* Ignore Pawn - this is to prevent objects shooting through the level or pawns 
	glitching on the top of small objects */
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	bStartActive = false;
	bIsActive = false;
	bAllowRespawn = true;
	RespawnDelay = 5.0f;
	RespawnDelayRange = 5.0f;

	SetReplicates(true);
}


void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

	RespawnPickup();
}


void ASPickupActor::OnUsed(APawn* InstigatorPawn)
{
	Super::OnUsed(InstigatorPawn);

	// Not active in level after pickup
	bIsActive = false;
	OnPickedUp();

	if (bAllowRespawn)
	{
		FTimerHandle RespawnTimerHandle;
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ASPickupActor::RespawnPickup, RespawnDelay + FMath::RandHelper(RespawnDelayRange), false);
	}
	else
	{
		Destroy();
	}
}


void ASPickupActor::OnRep_IsActive()
{
	if (bIsActive)
	{
		OnRespawned();
	}
	else
	{
		OnPickedUp();
	}
}


void ASPickupActor::OnPickedUp()
{
	if (MeshComp)
	{
		MeshComp->SetVisibility(false);
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


void ASPickupActor::OnRespawned()
{
	if (MeshComp)
	{
		MeshComp->SetVisibility(true);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ASPickupActor::RespawnPickup()
{
	bIsActive = true;
	OnRespawned();
}

void ASPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPickupActor, bIsActive);
}