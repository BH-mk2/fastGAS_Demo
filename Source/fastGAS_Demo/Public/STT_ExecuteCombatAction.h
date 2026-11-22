// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_ExecuteCombatAction.generated.h"

class ACombatCharacterBase;

USTRUCT()
struct FSTT_ExecuteCombatActionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACombatCharacterBase> Character = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag FollowUpEventTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag ReleaseEventTag;
};

USTRUCT(meta = (DisplayName = "Execute Combat Action", Category = "Combat"))
struct FASTGAS_DEMO_API FSTT_ExecuteCombatAction : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ExecuteCombatActionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
