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

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	ASC->AddLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Parry);
	GetWorld()->GetTimerManager().SetTimer(ParryTimerHandle, this, &UGA_Guard::ReleaseParry, ParrySec, false);

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

void UGA_Guard::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsReleasedParry)
	{
		ReleaseParry();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Guard::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Guard::OnGuardEndEventReceived(FGameplayEventData payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Guard::ReleaseParry()
{
	IsReleasedParry = false;
	ACombatCharacterBase* Character = GetCombatCharacter();
	if (!Character)
		return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!ASC)
		return;

	ASC->RemoveLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Parry);
}
