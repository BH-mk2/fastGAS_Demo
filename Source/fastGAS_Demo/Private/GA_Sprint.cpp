// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Sprint.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "CombatGameplayTags.h"

void UGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 終了を待つ
	auto WaitInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::CGTAG_EVENT_Command_Release,
		nullptr,
		false,
		true
	);
	WaitInputTask->EventReceived.AddDynamic(this, &UGA_Sprint::OnReleaseEventReceived);
	WaitInputTask->ReadyForActivation();


	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (SprintEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SprintEffectClass, GetAbilityLevel(), Context);
			if (SpecHandle.IsValid())
			{
				ActiveSprintEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActiveSprintEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(ActiveSprintEffectHandle);
		}
		ActiveSprintEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGA_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
	{
		if (DodgeStateTag.IsValid() && ASC->HasMatchingGameplayTag(DodgeStateTag))
		{
			if (!CancelAbilitiesWithTag.IsValid())
				return false;
			return ASC->HasMatchingGameplayTag(CancelWindowTag);
		}
	}
	return true;
}

void UGA_Sprint::OnReleaseEventReceived(FGameplayEventData payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
