// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Guard.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatGameplayTags.h"

void UGA_Guard::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACombatCharacterBase* Character = GetCombatCharacter();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		GuardMontage,
		PlayRate
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Guard::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Guard::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Guard::OnMontageEnded);

	MontageTask->ReadyForActivation();

	// ガード終了を待つ
	auto WaitInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::CGTAG_EVENT_Input_GuardEnd,
		nullptr,
		false,
		true
	);

	WaitInputTask->EventReceived.AddDynamic(this, &UGA_Guard::OnGuardEndEventReceived);
	WaitInputTask->ReadyForActivation();
}

void UGA_Guard::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Guard::OnGuardEndEventReceived(FGameplayEventData payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
