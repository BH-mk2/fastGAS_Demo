// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Stagger.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "CombatGameplayTags.h"

UGA_Stagger::UGA_Stagger()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = CombatGameplayTags::CGTAG_EVENT_Combat_PostureBroken;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	CancelAbilitiesWithTag.AddTag(CombatGameplayTags::CGTAG_STATE_Action);
}

void UGA_Stagger::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (StaggerEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaggerEffect, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			ActivateStaggerEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	UAbilityTask_WaitGameplayTagRemoved* WaitStaggerTagRemoved = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
		this, CombatGameplayTags::CGTAG_STATE_Action_Staggered);
	WaitStaggerTagRemoved->Removed.AddDynamic(this, &UGA_Stagger::OnStaggerTagRemoved);
	WaitStaggerTagRemoved->ReadyForActivation();

	if (!StaggerMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		StaggerMontage
	);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Stagger::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Stagger::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Stagger::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Stagger::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_Stagger::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (EffectOnEnd)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectOnEnd, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				ActivateStaggerEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UGA_Stagger::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Stagger::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Stagger::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Stagger::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Stagger::OnStaggerTagRemoved()
{
	MontageJumpToSection(RecoverSectionName);
}
