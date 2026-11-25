#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "SamuraiCharacter.h"
#include "STE_CombatContext.generated.h"

class ASamuraiCharacter;

USTRUCT()
struct FSTE_CombatContextInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ASamuraiCharacter> OwnerCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float AngleToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTagContainer AllowedAttackTags;

	UPROPERTY(EditAnywhere, Category = "Output")
	float HealthRatio = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float PostureRatio = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTagContainer OwnedTags;
};

USTRUCT(meta = (DisplayName = "Combat Context Evaluator"))
struct FASTGAS_DEMO_API FSTE_CombatContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTE_CombatContextInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	void UpdateContextData(FInstanceDataType& InstanceData) const;
};
