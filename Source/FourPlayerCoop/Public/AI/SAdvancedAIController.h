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

	UPROPERTY(EditDefaultsOnly, Category = "AI Weapon")
	TSubclassOf<class ASWeapon> AIWeaponClass;

	/* If there is a line of sight to current enemy start firing */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void ShootAtEnemy();

	void SetTargetEnemy(APawn* NewTarget);

	class ASCharacter* GetEnemy() const;

	/** Returns BehaviourTreeCompnent SubObjects **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorTreeComp() const { return BehaviorTreeComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
