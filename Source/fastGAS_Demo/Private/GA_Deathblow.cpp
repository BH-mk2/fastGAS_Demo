// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Deathblow.h"
#include "SamuraiCharacter.h"
#include "DeathblowAction.h"

void UGA_Deathblow::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ASamuraiCharacter* Attacker = Cast<ASamuraiCharacter>(GetAvatarActorFromActorInfo());
	if (!Attacker)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}



	auto TargetActors= UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TriggerEventData->TargetData);
	if (TargetActors.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ASamuraiCharacter* Victim= Cast<ASamuraiCharacter>(TargetActors[0]);
	ActiveDeathblowAction = Victim->GetDeathblowAction();
	if (!Victim || !ActiveDeathblowAction.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveDeathblowAction->OnFinished.AddDynamic(this, &UGA_Deathblow::OnDeathblowActionFinished);

	ActiveDeathblowAction->Execute(Attacker, Victim);
}

void UGA_Deathblow::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActiveDeathblowAction.IsValid())
	{
		ActiveDeathblowAction->OnFinished.RemoveDynamic(this, &UGA_Deathblow::OnDeathblowActionFinished);
		ActiveDeathblowAction = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Deathblow::OnDeathblowActionFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

