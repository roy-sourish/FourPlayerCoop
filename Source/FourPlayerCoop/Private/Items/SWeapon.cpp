// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SWeapon.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/SAdvancedAIController.h"
#include "AI/SAdvancedAI.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "FourPlayerCoop/STypes.h"
#include "FourPlayerCoop/FourPlayerCoop.h"

ASWeapon::ASWeapon()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = Mesh;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetReplicates(true);
	bNetUseOwnerRelevancy = true;

	bIsEquipped = false;
	MuzzleAttachPoint = TEXT("MuzzleSocket");
	StorageSlot = EInventorySlot::Primary;
	CurrentState = EWeaponState::Idle;

	ShotsPerMinute = 700;
	StartAmmo = 999;
	MaxAmmo = 999;
	MaxAmmoPerClip = 30;
	NoEquipAnimDuration = 0.5f;
	NoAnimReloadDuration = 1.5f;

}


void ASWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Setup Configuration
	TimeBetweenShots = 60.0f / ShotsPerMinute;
	CurrentAmmo = FMath::Min(StartAmmo, MaxAmmo);
	CurrentAmmoInClip = FMath::Min(MaxAmmoPerClip, StartAmmo);
}


void ASWeapon::SetOwningPawn(ASCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		// Net Owner for RPC calls
		SetOwner(NewOwner);
	}
}


void ASWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}


USkeletalMeshComponent* ASWeapon::GetWeaponMesh() const
{
	return Mesh;
}


void ASWeapon::StartFire()
{
	// Push to server if client 
	if (!HasAuthority())
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}


void ASWeapon::StopFire()
{
	if (!HasAuthority())
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}


EWeaponState ASWeapon::GetCurrentState() const
{
	return CurrentState;
}


bool ASWeapon::CanFire() const
{
	bool bPawnCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOK = (CurrentState == EWeaponState::Idle) || CurrentState == EWeaponState::Firing;
	return bPawnCanFire && bStateOK && !bPendingReload;
}


FVector ASWeapon::GetAdjustedAim() const
{
	APawn* MyInstigator = GetInstigator();

	ASPlayerController* const PC = MyInstigator ? Cast<ASPlayerController>(MyInstigator->Controller) : nullptr;
	FVector FinalAim = FVector::ZeroVector;

	// If we have a player controller use it for the aim
	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;

		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (MyInstigator)
	{
		// Now see if we have an AI controller - we will want to get the aim from there if we do
		ASAdvancedAIController* AIController = MyPawn ? Cast<ASAdvancedAIController>(MyPawn->Controller) : nullptr;
		if (AIController != nullptr)
		{
			FinalAim = AIController->GetControlRotation().Vector();
		}
		else
		{
			FinalAim = MyInstigator->GetBaseAimRotation().Vector();
		}
		
	}

	return FinalAim;
}


FVector ASWeapon::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	ASPlayerController* PC = MyPawn ? Cast<ASPlayerController>(MyPawn->Controller) : nullptr;
	ASAdvancedAIController* AIPC = MyPawn ? Cast<ASAdvancedAIController>(MyPawn->Controller) : nullptr;

	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		FRotator DummyRot;
		PC->GetPlayerViewPoint(OutStartTrace, DummyRot);

		/* Adjust the ray so there is nothing blocking the ray between the camera and the pawn and calculate the distance from adjusted start. */
		OutStartTrace = OutStartTrace + AimDir * (FVector::DotProduct((GetInstigator()->GetActorLocation() - OutStartTrace), AimDir));
	}
	else if (AIPC)
	{
		OutStartTrace = GetMuzzleLocation();
	}

	return OutStartTrace;
}


FHitResult ASWeapon::WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const
{
	FCollisionQueryParams TraceParams(TEXT("WeaponTrace"), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}


void ASWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload())
			{
				NewState = EWeaponState::Reloading;
			}
			else
			{
				NewState = CurrentState;
			}
		}
		else if (!bPendingReload && bWantsToFire && CanFire())
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}


void ASWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}


void ASWeapon::HandleFiring()
{
	if (CurrentAmmoInClip > 0 && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();

			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
		}

		/* Reload after firing last round */
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (!HasAuthority())
		{
			ServerHandleFiring();
		}

		bRefiring = (CurrentState == EWeaponState::Firing) && (TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ASWeapon::HandleFiring, TimeBetweenShots, false);
		}
	}

	if (MyPawn)
	{
		MyPawn->MakePawnNoise(1.0f);
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}


void ASWeapon::OnBurstStarted()
{
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0.0f && TimeBetweenShots > 0.0f && LastFireTime + TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ASWeapon::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}


void ASWeapon::OnBurstFinished()
{
	BurstCounter = 0;

	if (GetNetMode() == NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}


void ASWeapon::ServerStartFire_Implementation()
{
	StartFire();
}


bool ASWeapon::ServerStartFire_Validate()
{
	return true;
}


void ASWeapon::ServerStopFire_Implementation()
{
	StopFire();
}


bool ASWeapon::ServerStopFire_Validate()
{
	return true;
}


void ASWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0) && CanFire();

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		UseAmmo();
		BurstCounter++;
	}
}


bool ASWeapon::ServerHandleFiring_Validate()
{
	return true;
}


FVector ASWeapon::GetMuzzleLocation() const
{
	return Mesh->GetSocketLocation(MuzzleAttachPoint);
}


UAudioComponent* ASWeapon::PlayWeaponSound(USoundBase* SoundToPlay)
{
	UAudioComponent* AC = nullptr;
	if (SoundToPlay && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(SoundToPlay, MyPawn->GetRootComponent());
	}

	return AC;
}


void ASWeapon::SimulateWeaponFire()
{
	if (MuzzleFX)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh, MuzzleAttachPoint);
	}

	if (!bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	PlayWeaponSound(FireSound);
}


void ASWeapon::StopSimulatingWeaponFire()
{
	if (bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}
}


void ASWeapon::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;
}


void ASWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		/* By passing true we do not push back to the server and execute it locally */
		StartReload(true);
	}
	else
	{
		StopSimulateReload();
	}
}


bool ASWeapon::CanReload()
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	bool bGotAmmo = (CurrentAmmoInClip < MaxAmmoPerClip) && ((CurrentAmmo - CurrentAmmoInClip) > 0);
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return (bCanReload && bGotAmmo && bStateOKToReload);
}


void ASWeapon::ServerStartReload_Implementation()
{
	StartReload();
}


bool ASWeapon::ServerStartReload_Validate()
{
	return true;
}


void ASWeapon::StartReload(bool bFromReplication)
{
	// Push request to server 
	if (!bFromReplication && !HasAuthority())
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(ReloadAnim);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &ASWeapon::StopSimulateReload, AnimDuration, false);
		
		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ASWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}


void ASWeapon::StopSimulateReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}


void ASWeapon::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(MaxAmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);
	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}
}


int32 ASWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}


int32 ASWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}


int32 ASWeapon::GetMaxAmmoPerClip() const
{
	return MaxAmmoPerClip;
}


int32 ASWeapon::GetMaxAmmo() const
{
	return MaxAmmo;
}


ASCharacter* ASWeapon::GetPawnOwner() const
{
	return MyPawn;
}

void ASWeapon::OnEnterInventory(ASCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn(StorageSlot);
}


void ASWeapon::OnLeaveInventory()
{
	if (HasAuthority())
	{
		SetOwningPawn(nullptr);
	}

	if (IsWeaponAttachedToPawn())
	{
		OnUnEquip();
	}

	DetachMeshFromPawn();
}


void ASWeapon::AttachMeshToPawn(EInventorySlot Slot)
{
	if (MyPawn)
	{
		DetachMeshFromPawn();

		USkeletalMeshComponent* PawnMesh = MyPawn->GetMesh();

		FName AttachPoint = MyPawn->GetInventoryAttachPoints(Slot);

		Mesh->SetHiddenInGame(false);
		Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
	}
}


void ASWeapon::DetachMeshFromPawn()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetHiddenInGame(true);
}


void ASWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();

	if (MyPawn)
	{
		// Try to reload empty clip
		if (MyPawn->IsLocallyControlled() && CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}
	}
}


bool ASWeapon::IsEquipped() const
{
	return bIsEquipped;
}


bool ASWeapon::IsWeaponAttachedToPawn()
{
	return bIsEquipped || bPendingEquip;
}


void ASWeapon::OnEquip(bool bPlayAnimation)
{
	bPendingEquip = true;
	DetermineWeaponState();

	if (bPlayAnimation)
	{
		float Duration = PlayWeaponAnimation(EquipAnim);
		if (Duration <= 0.0f)
		{
			// Failsafe just in case if animation is missing 
			Duration = NoEquipAnimDuration;
		}
		EquipStartTime = GetWorld()->TimeSeconds;
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(EquipFinishedTimerHandle, this, &ASWeapon::OnEquipFinished, Duration, false);
	}
	else
	{
		OnEquipFinished();
	}

	// Play EquipSound
	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}


void ASWeapon::OnUnEquip()
{
	bIsEquipped = false;
	StopFire();

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(EquipFinishedTimerHandle);
	}

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	DetermineWeaponState();
}


void ASWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}


float ASWeapon::PlayWeaponAnimation(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		if (AnimMontage)
		{
			Duration = MyPawn->PlayAnimMontage(AnimMontage);
		}
	}

	return Duration;
}


void ASWeapon::StopWeaponAnimation(UAnimMontage* AnimMontage)
{
	if (MyPawn)
	{
		if (AnimMontage)
		{
			MyPawn->StopAnimMontage(AnimMontage);
		}
	}
}


void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWeapon, MyPawn);

	DOREPLIFETIME_CONDITION(ASWeapon, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASWeapon, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASWeapon, CurrentAmmoInClip, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASWeapon, bPendingReload, COND_SkipOwner);
}