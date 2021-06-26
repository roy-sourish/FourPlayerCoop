// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAdvancedAIController.h"
#include "AI/SAdvancedAI.h"
#include "Items/SWeapon.h"

/* AI specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


ASAdvancedAIController::ASAdvancedAIController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	TargetEnemyKeyName = "TargetEnemy";

	/* Initialize Player State so we can assign team index to AI */
	bWantsPlayerState = true;
}


void ASAdvancedAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASAdvancedAI* AIBot = Cast<ASAdvancedAI>(InPawn);
	if (AIBot)
	{
		if (ensure(AIBot->BehaviorTree->BlackboardAsset))
		{
			BlackboardComp->InitializeBlackboard(*AIBot->BehaviorTree->BlackboardAsset);
		}

		BehaviorTreeComp->StartTree(*AIBot->BehaviorTree);

		// Spawn Weapon 
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASWeapon* AIWeapon = GetWorld()->SpawnActor<ASWeapon>(AIWeaponClass, SpawnParams);
		AIBot->AddWeapon(AIWeapon);
	}
}


void ASAdvancedAIController::OnUnPossess()
{
	Super::OnUnPossess();

	BehaviorTreeComp->StopTree();
}


void ASAdvancedAIController::SetTargetEnemy(APawn* NewTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetEnemyKeyName, NewTarget);
	}
}


ASCharacter* ASAdvancedAIController::GetEnemy() const
{
	if (BlackboardComp)
	{
		return Cast<ASCharacter>(BlackboardComp->GetValueAsObject(TargetEnemyKeyName));
	}

	return nullptr;
}


void ASAdvancedAIController::ShootAtEnemy()
{
	ASAdvancedAI* MyBot = Cast<ASAdvancedAI>(GetPawn());
	ASWeapon* MyWeapon = MyBot ? MyBot->GetCurrentWeapon() : NULL;

	if (MyWeapon == NULL)
	{
		return;
	}

	bool bCanShoot = false;
	ASCharacter* Enemy = GetEnemy();

	if (Enemy && (Enemy->IsAlive()) && (MyWeapon->GetCurrentAmmo() > 0) && (MyWeapon->CanFire() == true))
	{
		if (LineOfSightTo(Enemy, MyBot->GetActorLocation()))
		{
			bCanShoot = true;
		}
	}

	if (bCanShoot)
	{
		MyBot->StartWeaponFire();
	}
	else
	{
		MyBot->StopWeaponFire();
	}
}

