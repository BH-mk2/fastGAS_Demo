// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Dodge.h"

#include "EnhancedPlayerInput.h"
#include "CombatCharacterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UGA_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 移動入力に従ってアニメーションを再生する
	// 移動入力をしてない場合、後ろ回避用を再生
	// 移動入力をした場合、キャラの向きを入力した方向に変えて前回避用を再生

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

	UEnhancedPlayerInput* EPI = Cast<UEnhancedPlayerInput>(PC->PlayerInput);
	if (!EPI)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector DesiredDir = Character->GetDesiredMoveDirection();

	UAnimMontage*const SelectedMontage = (DesiredDir.IsNearlyZero()) ? BackwardDodgeMontage : ForwardDodgeMontage;

	if (!DesiredDir.IsNearlyZero())
	{
		Character->bUseControllerRotationYaw = false;
		Character->SetActorRotation(DesiredDir.Rotation());
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SelectedMontage,
		PlayRate
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Dodge::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Dodge::OnMontageEnded);

	MontageTask->ReadyForActivation();
}

void UGA_Dodge::OnMontageEnded()
{
	if (ACombatCharacterBase* Character = GetCombatCharacter())
	{
		Character->bUseControllerRotationYaw = true;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
