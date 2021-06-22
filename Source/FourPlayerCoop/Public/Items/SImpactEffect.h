// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SImpactEffect.generated.h"

UCLASS(Abstract, Blueprintable)
class FOURPLAYERCOOP_API ASImpactEffect : public AActor
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this actor's properties
	ASImpactEffect();

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* DefaultImpactFX;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* PlayerFleshFX;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* DefaultSound;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* PlayerFleshSound;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	UMaterial* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	float DecalSize;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	float DecalLifeSpan;

	FHitResult SurfaceHit;

protected:

	virtual void PostInitializeComponents() override;

	class UParticleSystem* GetImpactFX(EPhysicalSurface SurfaceType) const;

	class USoundCue* GetImpactSound(EPhysicalSurface SurfaceType) const;
};
