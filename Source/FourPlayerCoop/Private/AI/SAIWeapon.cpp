// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAIWeapon.h"

// Sets default values
ASAIWeapon::ASAIWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASAIWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASAIWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

