// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/SBaseCharacter.h"
#include "SCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

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
};
