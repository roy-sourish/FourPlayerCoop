// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAdvancedAIController.h"
#include "AI/SAdvancedAI.h"
#include "Player/SBaseCharacter.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerState.h"
#include "Items/SWeapon.h"
#include "EngineUtils.h"

/* AI specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


ASAdvancedAIController::ASAdvancedAIController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	TargetEnemyKeyName = "TargetEnemy";
	LastKnownPositionKeyName = "LastKnownPosition";

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
		SetFocus(NewTarget);
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


void ASAdvancedAIController::SetLastKnownPosition(FVector NewPosition)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(LastKnownPositionKeyName, NewPosition);
	}
}


FVector ASAdvancedAIController::GetLastKnownPosition() const
{
	if (BlackboardComp)
	{
		return (BlackboardComp->GetValueAsVector(LastKnownPositionKeyName));
	}

	return FVector::ZeroVector;
}


void ASAdvancedAIController::FindClosestEnemy()
{
	APawn* MyBot = GetPawn();
	if (MyBot == nullptr)
	{
		return;
	}

	bool bIsEnemy = false;

	const FVector MyLoc = MyBot->GetActorLocation();
	float BestDistSq = MAX_FLT;
	ASCharacter* BestPawn = nullptr;

	for (ASCharacter* TestPawn : TActorRange<ASCharacter>(GetWorld()))
	{
		if (TestPawn->IsAlive() && TestPawn->IsEnemyFor(this))
		{
			const float DistSq = (TestPawn->GetActorLocation() - MyLoc).SizeSquared();
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPawn = TestPawn;
			}
		}
	}

	if (BestPawn)
	{
		//SetTargetEnemy(BestPawn);
		SetLastKnownPosition(BestPawn->GetActorLocation());
	}
}


bool ASAdvancedAIController::FindClosestEnemyWithLOS(ASCharacter* ExcludedEnemy)
{
	bool bGotEnemy = false;
	APawn* MyBot = GetPawn();
	if (MyBot)
	{
		const FVector MyLoc = MyBot->GetActorLocation();
		float BestDistSq = FLT_MAX;
		ASCharacter* BestPawn = nullptr;

		for (ASCharacter* TestPawn : TActorRange<ASCharacter>(GetWorld()))
		{
			if (TestPawn != ExcludedEnemy && TestPawn->IsAlive() && TestPawn->IsEnemyFor(this))
			{
				if (HasWeaponLOSToEnemy(TestPawn, true))
				{
					const float DistSq = (TestPawn->GetActorLocation() - MyLoc).SizeSquared();
					if (DistSq < BestDistSq)
					{
						BestDistSq = DistSq;
						BestPawn = TestPawn;
					}
				}
			}
		}

		if (BestPawn)
		{
			SetTargetEnemy(BestPawn);
			bGotEnemy = true;
		}
	}

	return bGotEnemy;
}


bool ASAdvancedAIController::HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const
{
	ASAdvancedAI* MyBot = Cast<ASAdvancedAI>(GetPawn());
	
	bool bHasLOS = false;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(AIWeaponLOSTrace), true, GetPawn());
	TraceParams.bReturnPhysicalMaterial = true;

	FVector StartLocation = MyBot->GetActorLocation();
	StartLocation.Z += GetPawn()->BaseEyeHeight;	// Look from eyes
	const FVector EndLocation = InEnemyActor->GetActorLocation();

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_WEAPON, TraceParams);

	if (Hit.bBlockingHit)
	{
		// There's a blocking hit - check if it's our enemy 
		AActor* HitActor = Hit.GetActor();
		if (Hit.GetActor())
		{
			if (HitActor == InEnemyActor)
			{
				bHasLOS = true;
			}
			else if (bAnyEnemy)
			{
				// It's not out actor, maybe it's still an enemy?
				ASCharacter* HitChar = Cast<ASCharacter>(HitActor);
				if (HitChar)
				{
					ASPlayerState* HitPlayerState = Cast<ASPlayerState>(HitChar->GetPlayerState());
					ASPlayerState* MyPlayerState = Cast<ASPlayerState>(MyBot->GetPlayerState());

					if (HitPlayerState->GetTeamNumber() != MyPlayerState->GetTeamNumber())
					{
						bHasLOS = true;
					}
				}
			}
		}
	}

	return bHasLOS;
}


void ASAdvancedAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	// Look towards Focus
	FVector FoculPoint = GetFocalPoint();
	if (!FoculPoint.IsZero() && GetPawn())
	{
		FVector Direction = FoculPoint - GetPawn()->GetActorLocation();
		FRotator NewControlRotation = Direction.Rotation();

		NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);
		SetControlRotation(NewControlRotation);

		APawn* const P = GetPawn();
		if (P && bUpdatePawn)
		{
			P->FaceRotation(NewControlRotation, DeltaTime);
		}
	}
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

