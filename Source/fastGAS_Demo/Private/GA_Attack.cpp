// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Attack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UGA_Attack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACombatCharacterBase* Character = GetCombatCharacter();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerController* PC = Cast<APlayerController>(CurrentActorInfo->PlayerController);
	if (!PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		PlayRate
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Attack::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Attack::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Attack::OnMontageEnded);

	MontageTask->ReadyForActivation();
}

void UGA_Attack::OnMontageEnded()
{
	if (ACombatCharacterBase* Character = GetCombatCharacter())
	{
		Character->bUseControllerRotationYaw = true;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

