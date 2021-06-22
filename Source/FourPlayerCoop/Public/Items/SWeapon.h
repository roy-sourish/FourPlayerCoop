// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FourPlayerCoop/STypes.h"
#include "SWeapon.generated.h"

enum class EInventorySlot : uint8;
class ASCharacter;

UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Equipping,
	Reloading
};


UCLASS(ABSTRACT, Blueprintable)
class FOURPLAYERCOOP_API ASWeapon : public AActor
{
	GENERATED_BODY()

	virtual void PostInitializeComponents() override;

	bool bIsEquipped;

	bool bPendingEquip;

	float EquipDuration;

	float EquipStartTime;

	FTimerHandle EquipFinishedTimerHandle;

protected:	

	// Sets default values for this actor's properties
	ASWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USkeletalMeshComponent* Mesh;

	/* Character Socket to store this item at */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EInventorySlot StorageSlot;

	/* Pawn Owner */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_MyPawn)
	ASCharacter* MyPawn;

	UFUNCTION()
	void OnRep_MyPawn();

	virtual void OnEquipFinished();

	bool IsEquipped() const;

	bool IsWeaponAttachedToPawn();

public:

	/* Handles Weapon Equip and Un-Equip*/
	void OnEquip(bool bPlayAnimation);
	void OnUnEquip();

	/* Set Weapons Owning Pawn */
	void SetOwningPawn(ASCharacter* NewOwner);

	/* Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class ASCharacter* GetPawnOwner() const;

	virtual void OnEnterInventory(ASCharacter* NewOwner);

	virtual void OnLeaveInventory();

	/**
	* By default the weapon will attach to the hands if not specified in function call.
	* You can specify/override it during function call.
	*/
	void AttachMeshToPawn(EInventorySlot Slot = EInventorySlot::Hands);

	void DetachMeshFromPawn();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	FORCEINLINE EInventorySlot GetStorageSlot()
	{
		return StorageSlot;
	}

	/* Class to spawn when weapon is dropped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class ASWeaponPickup> WeaponPickupClass;





	/******************************************************************/
	/* Fire and Damage Handling                                       */
	/******************************************************************/

public:

	void StartFire();

	void StopFire();

	EWeaponState GetCurrentState() const;

protected:

	bool CanFire() const;

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;
	
	/* With PURE_VIRTUAL we skip implementing the function in SWeapon.cpp and can do this in SWeaponInstant.cpp */
	virtual void FireWeapon() PURE_VIRTUAL(ASWeapon::FireWeapon, );

private:

	bool bWantsToFire;

	bool bRefiring;

	EWeaponState CurrentState;

	float LastFireTime;

	/* Time between shots for repeating fire */
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ShotsPerMinute;
	
	FTimerHandle TimerHandle_HandleFiring;

	void DetermineWeaponState();

	void SetWeaponState(EWeaponState NewState);
	
	virtual void HandleFiring();

	void OnBurstStarted();

	void OnBurstFinished();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();

	void ServerStartFire_Implementation();
	
	bool ServerStartFire_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();

	void ServerStopFire_Implementation();

	bool ServerStopFire_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleFiring();

	void ServerHandleFiring_Implementation();

	bool ServerHandleFiring_Validate();

	
	/******************************************************************/
	/* Simulation and FX                                              */
	/******************************************************************/

private:

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* EquipSound;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/* Weapon Muzzle Socket Name */
	UPROPERTY(EditDefaultsOnly)
	FName MuzzleAttachPoint;

	bool bPlayingFireAnim;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	UFUNCTION()
	void OnRep_BurstCounter();
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	float PlayWeaponAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* AnimMontage);

	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	UAudioComponent* PlayWeaponSound(USoundBase* SoundToPlay);

	virtual void SimulateWeaponFire();
	
	virtual void StopSimulatingWeaponFire();



	/*********************************************************************/
	/* Ammo and Reloading                                                */
	/*********************************************************************/
private:
	
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* OutOfAmmoSound;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoAnimReloadDuration;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	void UseAmmo();

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/* Weapon Ammo to Spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnim;

	virtual void ReloadWeapon();

	UFUNCTION()
	void OnRep_Reload();

	bool CanReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();

	void ServerStartReload_Implementation();
	
	bool ServerStartReload_Validate();



public:

	virtual void StartReload(bool bFromReplication = false);

	virtual void StopSimulateReload();

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetCurrentAmmo() const;
	
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetMaxAmmoPerClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetMaxAmmo() const;
};
