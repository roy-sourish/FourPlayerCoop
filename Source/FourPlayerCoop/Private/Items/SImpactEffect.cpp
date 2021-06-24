// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SImpactEffect.h"
#include "Components/DecalComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "FourPlayerCoop/FourPlayerCoop.h"

// Sets default values
ASImpactEffect::ASImpactEffect()
{
	SetAutoDestroyWhenFinished(true);
	PrimaryActorTick.bCanEverTick = true;

	DecalLifeSpan = 10.0f;
	DecalSize = 16.0f;
}

void ASImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	/* Figure out what we hit. (SurfaceHit is set during actor instantiation in the weapon class */
	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	UParticleSystem* ImpactEffect = GetImpactFX(HitSurfaceType);
	if (ImpactEffect)
	{
		//UE_LOG(LogTemp, Error, TEXT("Impact"));
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, GetActorLocation(), GetActorRotation());
	}

	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (DecalMaterial)
	{
		FVector ImpactNormal = SurfaceHit.ImpactNormal;
		ImpactNormal.Normalize();

		/* Inverse to point towards the wall. 
		   Invert to get the correct orientation of the decal 
		   (pointing into the surface instead of away, messing with the normals, and lighting) */
		ImpactNormal = -ImpactNormal;

		FRotator RandomDecalRotation = ImpactNormal.ToOrientationRotator();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UDecalComponent* DecalComp = UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(DecalSize),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation,
			EAttachLocation::KeepWorldPosition, DecalLifeSpan);

		if (DecalComp)
		{
			DecalComp->SetFadeOut(DecalLifeSpan, 0.5f, false);
		}
	}
}

UParticleSystem* ASImpactEffect::GetImpactFX(EPhysicalSurface SurfaceType) const
{
	switch (SurfaceType)
	{
	case SURFACE_DEFAULT:
		return DefaultImpactFX;
	case SURFACE_BODY:
	case SURFACE_HEAD:
	case SURFACE_LIMB:
		return PlayerFleshFX;
	default:
		return nullptr;
	}
}


USoundCue* ASImpactEffect::GetImpactSound(EPhysicalSurface SurfaceType) const
{
	switch (SurfaceType)
	{
	case SURFACE_DEFAULT:
		return DefaultSound;
	case SURFACE_BODY:
	case SURFACE_HEAD:
	case SURFACE_LIMB:
		return PlayerFleshSound;
	default:
		return nullptr;
	}
}
