// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FourPlayerCoop/FourPlayerCoop.h"
#include "SAdvancedAIController.generated.h"


class UBehaviorTreeComponent;
class ASBaseCharacter;

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API ASAdvancedAIController : public AAIController
{
	GENERATED_BODY()

protected:

	ASAdvancedAIController();

	/* Called whenever a controller possesses a character bot */
	virtual void OnPossess(class APawn* InPawn) override;

	virtual void OnUnPossess() override;

	UBehaviorTreeComponent* BehaviorTreeComp;

	UBlackboardComponent* BlackboardComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetEnemyKeyName;

public:

	void SetTargetEnemy(APawn* NewTarget);

	/** Returns BehaviourTreeCompnent SubObjects **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorTreeComp() const { return BehaviorTreeComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
