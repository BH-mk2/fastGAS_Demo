// Fill out your copyright notice in the Description page of Project Settings.


#include "STT_ExecuteCombatAction.h"
#include "StateTreeExecutionContext.h"

#include "CombatCharacterBase.h"
#include "CombatGameplayTags.h"

EStateTreeRunStatus FSTT_ExecuteCombatAction::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.Character == nullptr || not InstanceData.AbilityTag.IsValid())
		return EStateTreeRunStatus::Failed;

	InstanceData.Character->TriggerAction(InstanceData.AbilityTag, InstanceData.FollowUpEventTag);
	if (!InstanceData.Character->IsAbilityActiveByTag(InstanceData.AbilityTag))
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_ExecuteCombatAction::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.Character == nullptr)
		return EStateTreeRunStatus::Failed;

	if (not InstanceData.Character->IsAbilityActiveByTag(InstanceData.AbilityTag))
		return EStateTreeRunStatus::Succeeded;

	return EStateTreeRunStatus::Running;
}

void FSTT_ExecuteCombatAction::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.Character
		&& InstanceData.ReleaseEventTag.IsValid()
		&& InstanceData.Character->IsAbilityActiveByTag(InstanceData.AbilityTag))
	{
		InstanceData.Character->ReleaseAction(InstanceData.AbilityTag, InstanceData.ReleaseEventTag);
	}
}
