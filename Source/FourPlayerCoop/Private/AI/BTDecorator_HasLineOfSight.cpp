// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_HasLineOfSight.h"
#include "AI/SAdvancedAIController.h"
#include "AI/SAdvancedAI.h"
#include "Player/SPlayerState.h"
#include "FourPlayerCoop/FourPlayerCoop.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"



UBTDecorator_HasLineOfSight::UBTDecorator_HasLineOfSight(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Has LOS To";
	// accept only actors and vectors 
	EnemyKey.AddObjectFilter(this, *NodeName, AActor::StaticClass());
	EnemyKey.AddVectorFilter(this, *NodeName);
}


bool UBTDecorator_HasLineOfSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	AAIController* MyController = OwnerComp.GetAIOwner();
	bool HasLOS = false;

	if (MyController && MyBlackboard)
	{
		auto MyID = MyBlackboard->GetKeyID(EnemyKey.SelectedKeyName);
		auto TargetKeyType = MyBlackboard->GetKeyType(MyID);

		FVector TargetLocation;
		bool bGotTarget = false;
		AActor* EnemyActor = NULL;
		if (TargetKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(MyID);
			EnemyActor = Cast<AActor>(KeyValue);
			if (EnemyActor)
			{
				TargetLocation = EnemyActor->GetActorLocation();
				bGotTarget = true;
			}
		}
		else if (TargetKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(MyID);
			bGotTarget = true;
		}

		if (bGotTarget == true)
		{
			if (LOSTrace(OwnerComp.GetOwner(), EnemyActor, TargetLocation) == true)
			{
				HasLOS = true;
			}
		}
	}

	return HasLOS;
}


bool UBTDecorator_HasLineOfSight::LOSTrace(AActor* InActor, AActor* InEmemyActor, const FVector& EndLocation) const
{
	ASAdvancedAIController* MyController = Cast<ASAdvancedAIController>(InActor);
	ASAdvancedAI* MyBot = MyController ? Cast<ASAdvancedAI>(MyController->GetPawn()) : nullptr;

	bool bHasLOS = false;
	if (MyBot)
	{
		// Perform trace to retrieve hit info 
		FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(AILosTrace), true, InActor);
		TraceParams.bReturnPhysicalMaterial = true;
		TraceParams.AddIgnoredActor(MyBot);

		const FVector StartLocation = MyBot->GetActorLocation();

		FHitResult Hit(ForceInit);
		GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_WEAPON, TraceParams);
		if (Hit.bBlockingHit == true)
		{
			// We hit something. If we have an actor supplied, just check if the hit actor is an enemy. If it is consider that 'has LOS'
			AActor* HitActor = Hit.GetActor();
			if (Hit.GetActor())
			{
				// If the hit is our target actor consider LOS 
				if (HitActor == InActor)
				{
					bHasLOS = true;
				}
				else
				{
					// Check the team of us against the team of the actor we hit if we are able. If they dont match good to go.
					ACharacter* HitChar = Cast<ACharacter>(HitActor);
					if (HitChar && (MyController->PlayerState) && (HitChar->GetPlayerState()))
					{
						ASPlayerState* HitPlayerState = Cast<ASPlayerState>(HitChar->GetPlayerState());
						ASPlayerState* MyPlayerState = Cast<ASPlayerState>(MyController->PlayerState);
						if (HitPlayerState && MyPlayerState)
						{
							if (HitPlayerState->GetTeamNumber() != MyPlayerState->GetTeamNumber())
							{
								bHasLOS = true;
							}
						}
					}

				}
			}
			else // we didn't hit an actor 
			{
				if (InEmemyActor == NULL)
				{
					// We were not given an actor - so check of the distance between what we hit and the target. If what we hit is further away than the target we should be able to hit our target.
					FVector HitDelta = Hit.ImpactPoint - StartLocation;
					FVector TargetDelta = EndLocation - StartLocation;
					if (TargetDelta.SizeSquared() < HitDelta.SizeSquared())
					{
						bHasLOS = true;
					}
				}
			}
		}

	}

	return bHasLOS;
}
