// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Guard.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "CombatGameplayTags.h"
#include "AbilitySystemInterface.h"

UGA_Guard::UGA_Guard()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

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

	// ガード中常時発動するGEを起動
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(PassiveGuardEffect, 1.0f, Context);
		PassiveGuardEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Guard::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Guard::OnMontageEnded);
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

	UAbilityTask_WaitInputRelease* WaitReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitReleaseTask->OnRelease.AddDynamic(this, &UGA_Guard::OnInputReleased);
	WaitReleaseTask->ReadyForActivation();

	// 攻撃を受けたときのタスク
	UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::CGTAG_EVENT_Combat_HitBlock,
		nullptr,
		false,
		true
	);

	WaitHitTask->EventReceived.AddDynamic(this, &UGA_Guard::OnHitBlockReceived);
	WaitHitTask->ReadyForActivation();

	// 弾きを成功させたときのタスク
	UAbilityTask_WaitGameplayEvent* WaitParryTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::CGTAG_EVENT_Combat_ParrySuccess,
		nullptr,
		false,
		true
	);

	WaitParryTask->EventReceived.AddDynamic(this, &UGA_Guard::OnParrySuccessReceived);
	WaitParryTask->ReadyForActivation();
}

void UGA_Guard::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsReleasedParry)
	{
		ReleaseParry();
	}

	{
		if (ACombatCharacterBase* Character = GetCombatCharacter())
		{
			if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
			{
				SourceASC->RemoveActiveGameplayEffect(PassiveGuardEffectHandle);
			}
		}
		PassiveGuardEffectHandle.Invalidate();

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

void UGA_Guard::OnInputReleased(float TImeHeld)
{
	MontageJumpToSection(EndSectionName);
}

void UGA_Guard::OnHitBlockReceived(FGameplayEventData Payload)
{
	MontageJumpToSection(HitSectionName);
}

void UGA_Guard::OnParrySuccessReceived(FGameplayEventData Payload)
{
	ACombatCharacterBase* Character = GetCombatCharacter();
	if (!Character)
		return;

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!SourceASC)
		return;

	AActor* TargetActor = const_cast<AActor*>(Payload.Instigator.Get());
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	if (!TargetASC)
		return;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(Character, Character);

	if (!ParryEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Guard] ParryEffect is not set on %s"), *GetName());
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(ParryEffect, 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Guard] Failed to MakeOutgoingSpec"));
		return;
	}
	// TODO: キャラ毎のパラメータに変更する
	SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Health, 0.0f);
	SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture, 10.0f);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}
