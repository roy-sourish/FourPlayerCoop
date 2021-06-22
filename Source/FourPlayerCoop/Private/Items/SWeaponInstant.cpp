// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SWeaponInstant.h"
#include "Player/SPlayerController.h"
#include "Player/SCharacter.h"
#include "Items/SDamageType.h"
#include "Items/SImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"
#include "FourPlayerCoop/FourPlayerCoop.h"
#include "DrawDebugHelpers.h"


ASWeaponInstant::ASWeaponInstant()
{
	WeaponRange = 15000;
	HitDamage = 30;

	AllowedViewDotHitDir = -1.0f;
	ClientSideHitLeeway = 200.0f;
	MinimumProjectileSpawnDistance = 800;
	TracerRoundInterval = 3;
}


void ASWeaponInstant::FireWeapon()
{
	const FVector AimDir = GetAdjustedAim();
	const FVector CameraPos = GetCameraDamageStartLocation(AimDir);
	const FVector EndPos = CameraPos + (AimDir * WeaponRange);

	/* Check for impact by tracing from the camera position */
	FHitResult Impact = WeaponTrace(CameraPos, EndPos);

	const FVector MuzzleOrigin = GetMuzzleLocation();

	FVector AdjustedAimDir = AimDir;

	if (Impact.bBlockingHit)
	{
		/* Adjust the shoot direction to hit crosshair */	
		AdjustedAimDir = (Impact.ImpactPoint - MuzzleOrigin).GetSafeNormal();

		/* Retrace with the new aim direction coming out of the weapon muzzle */	
		Impact = WeaponTrace(MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange));
		
		// Debug weapon trace 
		//DrawDebugLine(GetWorld(), MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange), FColor::Red, false, 2.0f, 0.0f, 1.0f);
	}
	else
	{
		/* Use the maximum distance as the adjusted direction */
		Impact.ImpactPoint = FVector_NetQuantize(EndPos);
	}

	// Process Instant Hit 
	ProcessInstantHit(Impact, MuzzleOrigin, AdjustedAimDir);
}


void ASWeaponInstant::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	float ActualHitDamage = HitDamage;

	USDamageType* DmgType = Cast<USDamageType>(DamageType->GetDefaultObject());
	UPhysicalMaterial* PhysMat = Impact.PhysMaterial.Get();

	if (PhysMat && DmgType)
	{
		if (PhysMat->SurfaceType == SURFACE_HEAD)
		{
			ActualHitDamage *= DmgType->GetHeadshotDamageModifier();
		}
		else if (PhysMat->SurfaceType == SURFACE_LIMB)
		{
			ActualHitDamage *= DmgType->GetLimbDamageModifier();
		}
	}

	FPointDamageEvent PointDamg;
	PointDamg.DamageTypeClass = DamageType;
	PointDamg.HitInfo = Impact;
	PointDamg.ShotDirection = ShootDir;
	PointDamg.Damage = ActualHitDamage;

	Impact.GetActor()->TakeDamage(PointDamg.Damage, PointDamg, MyPawn->Controller, this);
}


bool ASWeaponInstant::ShouldDealDamage(AActor* TestActor) const
{
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->HasAuthority() ||
			TestActor->GetTearOff())
		{
			return true;
		}
	}

	return false;
}


void ASWeaponInstant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	if (MyPawn && MyPawn->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// If we are a client and hit something that is controlled by server 
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// Notify the server of our local hit to validate and apply actual hit damage.
			ServerNotifyHit(Impact, ShootDir);
		}
		else if (Impact.GetActor() == nullptr)
		{
			if (Impact.bBlockingHit)
			{
				ServerNotifyHit(Impact, ShootDir);
			}
			else
			{
				ServerNotifyMiss(ShootDir);
			}
		}
	}

	// Process a confirmed Hit
	ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
}


void ASWeaponInstant::ProcessInstantHitConfirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	// Handle Damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact, ShootDir);
	}

	// Play FX on local client 
	if (HasAuthority())
	{
		HitImpactNotify = Impact.ImpactPoint;
	}

	// Play FX Locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		SimulateInstantHit(Impact.ImpactPoint);
	}
}


void ASWeaponInstant::ServerNotifyHit_Implementation(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir)
{
	// If we have an instigator, calculate the dot between the view and the shot
	if (GetInstigator() && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = GetMuzzleLocation();
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		const float ViewDotHitDir = FVector::DotProduct(GetInstigator()->GetViewRotation().Vector(), ViewDir);
		if (ViewDotHitDir > AllowedViewDotHitDir)
		{
			// TODO: Check for weapon state

			if (Impact.GetActor() == nullptr)
			{
				if (Impact.bBlockingHit)
				{
					ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
				}
			}
			// Assume it told the truth about static things because we don't move and the hit
			// usually doesn't have significant gameplay implications
			else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
			{
				ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
			}
			else
			{
				const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

				FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
				BoxExtent *= ClientSideHitLeeway;

				BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
				BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
				BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

				const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

				// If we are within client tolerance
				if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
					FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
					FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
				{
					ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
				}
			}
		}
	}
}


bool ASWeaponInstant::ServerNotifyHit_Validate(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}


void ASWeaponInstant::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir)
{
	const FVector Origin = GetMuzzleLocation();
	const FVector EndTrace = Origin + (ShootDir * WeaponRange);

	// Play on remote clients
	HitImpactNotify = EndTrace;

	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnTrailEffects(EndTrace);
	}
}


bool ASWeaponInstant::ServerNotifyMiss_Validate(FVector_NetQuantizeNormal ShootDir)
{
	return true;
}


void ASWeaponInstant::SimulateInstantHit(const FVector& ImpactPoint)
{
	const FVector MuzzleOrigin = GetMuzzleLocation();

	/* Adjust direction based on desired crosshair impact point and muzzle location */
	const FVector AimDir = (ImpactPoint - MuzzleOrigin).GetSafeNormal();

	const FVector EndTrace = MuzzleOrigin + (AimDir * WeaponRange);
	const FHitResult Impact = WeaponTrace(MuzzleOrigin, EndTrace);

	if (Impact.bBlockingHit)
	{
		SpawnImpactEffects(Impact);
		SpawnTrailEffects(Impact.ImpactPoint);
	}
	else
	{
		SpawnTrailEffects(EndTrace);
	}
}


void ASWeaponInstant::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		/* This function prepares an actor to spawn, but requires another call to finish the actual spawn progress. This allows manipulation of properties before entering into the level */
		ASImpactEffect* ImpactEffectActor = GetWorld()->SpawnActorDeferred<ASImpactEffect>(ImpactTemplate, FTransform(Impact.ImpactPoint.Rotation(), Impact.ImpactPoint));
		if (ImpactEffectActor)
		{
			ImpactEffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(ImpactEffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
		}
	}
}


void ASWeaponInstant::SpawnTrailEffects(const FVector& EndPoint)
{
	// Keep local count for effects
	BulletsShotCount++;

	const FVector Origin = GetMuzzleLocation();
	FVector ShootDir = EndPoint - Origin;

	// Only spawn if a minimum distance is satisfied.
	if (ShootDir.Size() < MinimumProjectileSpawnDistance)
	{
		return;
	}

	if (BulletsShotCount % TracerRoundInterval == 0)
	{
		if (TracerFX)
		{
			ShootDir.Normalize();
			UGameplayStatics::SpawnEmitterAtLocation(this, TracerFX, Origin, ShootDir.Rotation());
		}
	}
	else
	{
		// Only create trails FX by other players.
		ASCharacter* OwningPawn = GetPawnOwner();
		if (OwningPawn && OwningPawn->IsLocallyControlled())
		{
			return;
		}

		if (TrailFX)
		{
			UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
			if (TrailPSC)
			{
				TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
			}
		}
	}
}


void ASWeaponInstant::OnRep_HitLocation()
{
	// Played on all remote clients
	SimulateInstantHit(HitImpactNotify);
}


void ASWeaponInstant::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeaponInstant, HitImpactNotify, COND_SkipOwner);
}