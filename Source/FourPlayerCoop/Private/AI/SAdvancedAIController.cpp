// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAdvancedAIController.h"
#include "AI/SAdvancedAI.h"

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
