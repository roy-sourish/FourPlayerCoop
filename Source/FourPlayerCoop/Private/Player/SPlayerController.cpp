// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SPlayerController.h"
#include "Player/SPlayerCameraManager.h"

ASPlayerController::ASPlayerController(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set Default Classes to use 
	PlayerCameraManagerClass = ASPlayerCameraManager::StaticClass();
}