// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SUsableActor.generated.h"

UCLASS()
class FOURPLAYERCOOP_API ASUsableActor : public AActor
{
	GENERATED_BODY()
	
protected:	

	// Sets default values for this actor's properties
	ASUsableActor();

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

public:

	/* Player Looking At */	
	virtual void OnBeginFocus();

	/* No longer looking at */	
	virtual void OnEndFocus();

	virtual void OnUsed(APawn* InstigatorPawn);

	FORCEINLINE UStaticMeshComponent* GetMeshComponent() const
	{
		return MeshComp;
	}
};
