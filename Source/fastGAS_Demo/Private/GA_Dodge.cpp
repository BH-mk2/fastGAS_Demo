// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Dodge.h"

#include "EnhancedPlayerInput.h"
#include "CombatCharacterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Kismet/KismetSystemLibrary.h"

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
	UE_LOG(LogTemp, Warning, TEXT("UGA_Dodge: DesiredDir: %s"), *DesiredDir.ToCompactString());

	// TODO: 向きによって再生するモンタージュを変える
	// 1. 入力がない場合、後ろステップ
	// 2. 真横の場合、左右ステップ
	// 3. 前方±45度の場合、アクタの向きを入力方向に変えてその方向に向かって前ステップ
	UAnimMontage* const SelectedMontage = [&]() {
		if (DesiredDir.IsNearlyZero())
		{
			return BackwardDodgeMontage;
		}

		const FRotator DesiredRot = DesiredDir.Rotation();
		const float AngleDiff = (DesiredRot - Character->GetViewRotation()).GetNormalized().Yaw;

		// 前
		if (-45.0f <= AngleDiff && AngleDiff <= 45.0f)
		{
			return ForwardDodgeMontage;
		}
		// 左
		else if ((-180.0f+45.0f) < AngleDiff && AngleDiff < -45.0f)
		{
			return LeftDodgeMontage;
		}
		// 右
		else if ((180.0f-45.0f) > AngleDiff && AngleDiff > 45.0f)
		{
			return RightDodgeMontage;
		}
		// 後ろ
		else
		{
			return BackwardDodgeMontage;
		}
		}();

	if (!DesiredDir.IsNearlyZero())
	{
		if (SelectedMontage == ForwardDodgeMontage)
		{
			Character->SetActorRotation(DesiredDir.Rotation());
		}
		else if (SelectedMontage == LeftDodgeMontage || SelectedMontage == RightDodgeMontage)
		{
			// None
		}
	}

	const float PlayRate = [&]() {
		if (SelectedMontage == ForwardDodgeMontage)return PlayRateForward;
		else if (SelectedMontage == BackwardDodgeMontage)return PlayRateBackward;
		else if (SelectedMontage == LeftDodgeMontage)return PlayRateLeft;
		else if (SelectedMontage == RightDodgeMontage)return PlayRateRight;
		else return 1.0f;
		}();

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
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::OnMontageEnded);

	MontageTask->ReadyForActivation();
}

void UGA_Dodge::OnMontageEnded()
{
	UE_LOG(LogTemp, Warning, TEXT("[UGA_Dodge] OnMontageEnded Called!"));

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
