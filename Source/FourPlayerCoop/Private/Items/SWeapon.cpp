// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SWeapon.h"
#include "FourPlayerCoop/STypes.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


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

	SetReplicates(true);

	bNetUseOwnerRelevancy = true;

	StorageSlot = EInventorySlot::Primary;

	NoEquipAnimDuration = 0.5f;
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

	//


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

	bPendingEquip = false;
}


void ASWeapon::OnEquip(bool bPlayAnimation)
{
	bPendingEquip = true;
	//DetermineWeaponState();

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
}


void ASWeapon::OnUnEquip()
{
	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(EquipFinishedTimerHandle);
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
}