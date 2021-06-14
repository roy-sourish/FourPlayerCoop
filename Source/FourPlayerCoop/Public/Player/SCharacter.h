// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/SBaseCharacter.h"
#include "SCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ASUsableActor;
class ASWeapon;
enum class EInventorySlot : uint8;

UCLASS()
class FOURPLAYERCOOP_API ASCharacter : public ASBaseCharacter
{
	GENERATED_BODY()

	ASCharacter(const class FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	/* Spring Arm Component */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoomComp;

	/* Primary Player Camera */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComp;

public:

	FORCEINLINE UCameraComponent* GetCameraComponent()
	{
		return CameraComp;
	}

	/***********************************************************************/
	/* Movement															   */
	/***********************************************************************/

	void MoveForward(float Val);
	
	void MoveRight(float Val);

	/* Client Mapped to Input */
	void OnCrouchToggle();

	/* Client Mapped to Input */
	void OnJump();

	/* Client Mapped to Input */
	void OnStartSprinting();

	/* Client Mapped to Input */
	void OnStopSprinting();

	virtual void SetSprinting(bool NewSprinting) override;

	/* Is Character currently performing a jump action. Reset on land */
	UPROPERTY(Transient, Replicated)
	bool bIsJumping;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsInitiatedJump() const;

	void SetIsJumping(bool NewJumping);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetIsJumping(bool NewJumping);

	void ServerSetIsJumping_Implementation(bool NewJumping);
	
	bool ServerSetIsJumping_Validate(bool NewJumping);

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	/*************************************************************************/
	/* Targeting															 */
	/*************************************************************************/

	void OnStartTargeting();

	void OnStopTargeting();


	/*************************************************************************/
	/* Object Interaction													 */
	/*************************************************************************/

	/* Mapped to input */
	void Use();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUse();

	void ServerUse_Implementation();

	bool ServerUse_Validate();

	/* Max distance that the player can focus from */
	UPROPERTY(EditDefaultsOnly, Category = "ObjectInteraction")
	float MaxUseDistance;

	/* Perform RayTrace to find the closest usable actor */
	ASUsableActor* GetUsableInView() const;

	ASUsableActor* FocusedUsableActor;

	/* True only in the first frame when focused on a new actor */
	bool bHasNewFocus;


	/*******************************************************************/
	/* Weapon and Inventory											   */
	/*******************************************************************/

private:

	// Weapon Attach Points -

	/* Attach point for Active weapon/item in hands */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName WeaponAttachPoint;

	/* Attach point for Secondary Weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName SecondaryAttachPoint;

	/* Attach point for primary weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName SpineAttachPoint;

	/* Mapped to input */
	void OnEquipPrimaryWeapon();

	/* Mapped to input */
	void OnEquipSecondaryWeapon();


public:

	/* All Weapons and items the player currently holds */
	/* NOTE: GameMode will populate this Inventory array and spawn default weapons of the player on spawn */
	UPROPERTY(Transient, Replicated)
	TArray<ASWeapon*> Inventory;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	ASWeapon* CurrentWeapon;

	ASWeapon* PreviousWeapon;

	void SetCurrentWeapon(ASWeapon* NewWeapon, ASWeapon* LastWeapon = nullptr);

	void EquipWeapon(ASWeapon* Weapon);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(ASWeapon* Weapon);

	void ServerEquipWeapon_Implementation(ASWeapon* Weapon);
	
	bool ServerEquipWeapon_Validate(ASWeapon* Weapon);

	UFUNCTION()
	void OnRep_CurrentWeapon(ASWeapon* LastWeapon);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	ASWeapon* GetCurrentWeapon() const;

	/* Return socket name for weapon attachment */
	FName GetInventoryAttachPoints(EInventorySlot Slot) const;

	/* Adds weapon to inventory */
	void AddWeapon(ASWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SwapToNewWeaponMesh();
};
