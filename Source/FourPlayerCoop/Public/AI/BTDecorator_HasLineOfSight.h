// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasLineOfSight.generated.h"

/**
 * 
 */
UCLASS()
class FOURPLAYERCOOP_API UBTDecorator_HasLineOfSight : public UBTDecorator
{
	GENERATED_BODY()

	UBTDecorator_HasLineOfSight(const FObjectInitializer& ObjectInitializer);

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
protected:

	UPROPERTY(EditAnywhere, Category = Condition)
	struct FBlackboardKeySelector EnemyKey;

private:

	bool LOSTrace(AActor* InActor, AActor* InEmemyActor, const FVector& EndLocation) const;
};
