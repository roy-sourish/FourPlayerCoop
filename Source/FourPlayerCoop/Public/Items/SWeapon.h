// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FourPlayerCoop/STypes.h"
#include "SWeapon.generated.h"

enum class EInventorySlot : uint8;
class ASCharacter;

UCLASS(ABSTRACT, Blueprintable)
class FOURPLAYERCOOP_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
private:

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

public:

	/* Handles Weapon Equip and Un-Equip*/
	void OnEquip(bool bPlayAnimation);
	void OnUnEquip();

	/* Set Weapons Owning Pawn */
	void SetOwningPawn(ASCharacter* NewOwner);

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


	/******************************************************************/
	/* Simulation and FX                                              */
	/******************************************************************/

private:

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* EquipAnim;
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	float PlayWeaponAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* AnimMontage);
};
