// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SUsableActor.h"

// Sets default values
ASUsableActor::ASUsableActor()
{
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

}


void ASUsableActor::OnBeginFocus()
{
    MeshComp->SetRenderCustomDepth(true);
}


void ASUsableActor::OnEndFocus()
{
    MeshComp->SetRenderCustomDepth(false);
}


void ASUsableActor::OnUsed(APawn* InstigatorPawn)
{
    // Overriden by SPickupActor
    UE_LOG(LogTemp, Warning, TEXT("OnUsed() -> Actor is Usable!"));
}
