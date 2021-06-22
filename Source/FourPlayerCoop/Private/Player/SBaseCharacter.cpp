// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SBaseCharacter.h"
#include "Components/SCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "World/SGameMode.h"
#include "Items/SDamageType.h"
#include "Kismet/GameplayStatics.h"
#include "FourPlayerCoop/STypes.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASBaseCharacter::ASBaseCharacter(const class FObjectInitializer& ObjectInitializer)
	/* Override the movement class from the base class to our own to support multiple speeds (eg. sprinting) */
	:Super(ObjectInitializer.SetDefaultSubobjectClass<USCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{

	Health = 100;

	SprintSpeedModifier = 2.5f;
	TargetingSpeedModifier = 0.5f;

	/* Don't collide with camera checks to keep 3rd Person Camera at position when bots or other players are standing behind us */
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}


bool ASBaseCharacter::IsSprinting() const
{
	return bWantsToRun && !IsTargeting() && !GetVelocity().IsZero()
		// Don't allow sprinting while strafing or standing still.
		// (1 is forward and -1 is backward, while near 0 sideways or standing still)
		// Changing the value to 0.1 will allow diagonal sprinting.
		&& (FVector::DotProduct(GetVelocity().GetSafeNormal2D(), GetActorRotation().Vector()) > 0.8f);
}


void ASBaseCharacter::SetSprinting(bool NewSprinting)
{
	bWantsToRun = NewSprinting;

	if (bIsCrouched)
	{
		UnCrouch();
	}

	if (!HasAuthority())
	{
		ServerSetSprinting(NewSprinting);
	}
}


float ASBaseCharacter::GetSprintSpeedModifier() const
{
	return SprintSpeedModifier;
}


void ASBaseCharacter::ServerSetSprinting_Implementation(bool NewSprinting)
{
	SetSprinting(NewSprinting);
}


bool ASBaseCharacter::ServerSetSprinting_Validate(bool NewSprinting)
{
	return true;
}


void ASBaseCharacter::SetTargeting(bool NewTargeting)
{
	bIsTargeting = NewTargeting;

	if (!HasAuthority())
	{
		ServerSetTargeting(NewTargeting);
	}
}


void ASBaseCharacter::ServerSetTargeting_Implementation(bool NewTargeting)
{
	SetTargeting(NewTargeting);
}


bool ASBaseCharacter::ServerSetTargeting_Validate(bool NewTargeting)
{
	return true;
}


bool ASBaseCharacter::IsTargeting() const
{
	return bIsTargeting;
}


float ASBaseCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}


FRotator ASBaseCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLs = AimDirLS.Rotation();
	
	return AimRotLs;
}


float ASBaseCharacter::GetMaxHealth() const
{
	// Retrieve the default value of the health property that is assigned on instantiation.
	return GetClass()->GetDefaultObject<ASBaseCharacter>()->Health;
}


float ASBaseCharacter::GetHealth() const
{
	return Health;
}


bool ASBaseCharacter::IsAlive() const
{
	return Health > 0;
}


float ASBaseCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}

	/* Modify based based on gametype rules */
	ASGameMode* MyGameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
	Damage = MyGameMode ? MyGameMode->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : Damage;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			bool bCanDie = true;

			/* Check the damagetype, always allow dying if the cast fails, otherwise check the property if player can die from damagetype */
			if (DamageEvent.DamageTypeClass)
			{
				USDamageType* DmgType = Cast<USDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
				bCanDie = (DmgType == nullptr || (DmgType && DmgType->GetCanDieFrom()));
			}

			if (bCanDie)
			{
				Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
			}
			else
			{
				/* Player cannot die from this damage type, set hitpoints to 1.0 */
				Health = 1.0f;
			}
		}
		else
		{
			/* Shorthand for - if x != null pick1 else pick2 */
			APawn* Pawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
			PlayHit(ActualDamage, DamageEvent, Pawn, DamageCauser, false);
		}
	}

	return ActualDamage;
}


bool ASBaseCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	/* Check if character is already dying, destroyed or if we have authority */
	if (bIsDying ||
		IsPendingKill() ||
		!HasAuthority() ||
		GetWorld()->GetAuthGameMode() == NULL)
	{
		return false;
	}

	return true;
}


bool ASBaseCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	/* Fallback to default DamageType if none is specified */
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	/* Notify the gamemode we got killed for scoring and game over state */
	AController* KilledPlayer = Controller ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<ASGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}


void ASBaseCharacter::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	SetReplicateMovement(true);
	TearOff();
	bIsDying = true;

	PlayHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

	DetachFromControllerPendingDestroy();

	/* Disable all collision on capsule */
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	USkeletalMeshComponent* Mesh3P = GetMesh();
	if (Mesh3P)
	{
		Mesh3P->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	SetActorEnableCollision(true);

	SetRagdollPhysics();

	/* Apply physics impulse on the bone of the enemy skeleton mesh we hit (ray-trace damage only) */
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent PointDmg = *((FPointDamageEvent*)(&DamageEvent));
		{
			// TODO: Use DamageTypeClass->DamageImpulse
			Mesh3P->AddImpulseAtLocation(PointDmg.ShotDirection * 12000, PointDmg.HitInfo.ImpactPoint, PointDmg.HitInfo.BoneName);
		}
	}
	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent RadialDmg = *((FRadialDamageEvent const*)(&DamageEvent));
		{
			Mesh3P->AddRadialImpulse(RadialDmg.Origin, RadialDmg.Params.GetMaxRadius(), 100000 /*RadialDmg.DamageTypeClass->DamageImpulse*/, ERadialImpulseFalloff::RIF_Linear);
		}
	}
}


void ASBaseCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
	Die(Health, FDamageEvent(DmgType.GetClass()), NULL, NULL);
}


void ASBaseCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;
	USkeletalMeshComponent* Mesh3P = GetMesh();

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!Mesh3P || !Mesh3P->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		Mesh3P->SetAllBodiesSimulatePhysics(true);
		Mesh3P->SetSimulatePhysics(true);
		Mesh3P->WakeAllRigidBodies();
		Mesh3P->bBlendPhysics = true;

		bInRagdoll = true;
	}

	UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharacterComp)
	{
		CharacterComp->StopMovementImmediately();
		CharacterComp->DisableMovement();
		CharacterComp->SetComponentTickEnabled(false);
	}

	if (!bInRagdoll)
	{
		// Immediately hide the pawn
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}


void ASBaseCharacter::PlayHit(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled)
{
	if (HasAuthority())
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, bKilled);
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		if (bKilled && SoundDeath)
		{
			UGameplayStatics::SpawnSoundAttached(SoundDeath, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
		}
		else if (SoundTakeHit)
		{
			UGameplayStatics::SpawnSoundAttached(SoundTakeHit, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
		}
	}
}


void ASBaseCharacter::ReplicateHit(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if (PawnInstigator == LastTakeHitInfo.PawnInstigator.Get() && LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass)
	{
		// Same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, ignore it
			return;
		}

		DamageTaken += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = DamageTaken;
	LastTakeHitInfo.PawnInstigator = Cast<ASBaseCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();
}


void ASBaseCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get(), LastTakeHitInfo.bKilled);
	}
}


void ASBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASBaseCharacter, bWantsToRun, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASBaseCharacter, bIsTargeting, COND_SkipOwner);
	
	DOREPLIFETIME(ASBaseCharacter, Health);
	DOREPLIFETIME(ASBaseCharacter, LastTakeHitInfo);
}