// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Attack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatGameplayTags.h"

#include "Kismet/KismetSystemLibrary.h"

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

	if (ComboSectionNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Attack] ComboSectionNames is Empty."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		PlayRate,
		ComboSectionNames[0]
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

	// コンボ入力を待つ
	auto WaitInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::CGTAG_EVENT_Input_Attack,
		nullptr,
		false,
		true
	);

	WaitInputTask->EventReceived.AddDynamic(this, &UGA_Attack::OnComboEventReceived);
	WaitInputTask->ReadyForActivation();

	// 先行入力の評価を行う
	auto WaitEvaluteBufferTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::CGTAG_EVENT_Input_Evaluate,
		nullptr,
		false,
		true
	);

	WaitEvaluteBufferTask->EventReceived.AddDynamic(this, &UGA_Attack::OnEvaluateBufferEventReceived);
	WaitEvaluteBufferTask->ReadyForActivation();
}

void UGA_Attack::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Attack::OnComboEventReceived(FGameplayEventData payload)
{
	UKismetSystemLibrary::PrintString(this, TEXT("[UGA_Attack] Called OnComboEventReceived."));

	ACombatCharacterBase* Character = GetCombatCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Atatck] Character is nothing"));
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Atatck] ASC is nothing"));
		return;
	}

	// キャンセル不可状態なら先行入力としておく
	if (!ASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Combat_Cancellable_Attack))
	{
		UKismetSystemLibrary::PrintString(this, TEXT("[UGA_Attack] Block Combo Input"));
		bComboBuffer = true;
		return;
	}

	ProcessNextCombo(*ASC);
}

void UGA_Attack::OnEvaluateBufferEventReceived(FGameplayEventData payload)
{
	if (!bComboBuffer)
	{
		return;
	}

	ACombatCharacterBase* Character = GetCombatCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Atatck] Character is nothing"));
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGA_Atatck] ASC is nothing"));
		return;
	}

	if (!ASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Combat_Cancellable_Attack))
	{
		return;
	}

	ProcessNextCombo(*ASC);
}

void UGA_Attack::ProcessNextCombo(UAbilitySystemComponent& ASC)
{
	// まだコンボできるか？ 
	if (!ComboSectionNames.IsValidIndex(ComboIndex + 1))
	{
		// できないので終了
		bComboBuffer = false;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 次の攻撃を再生
	++ComboIndex;
	MontageJumpToSection(ComboSectionNames[ComboIndex]);
	bComboBuffer = false;
}
