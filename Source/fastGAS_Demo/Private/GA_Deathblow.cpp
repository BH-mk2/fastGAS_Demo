// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Deathblow.h"

/*
void UGA_Deathblow::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	auto Actors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TriggerEventData->TargetData);
	if (Actors.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* TargetActor = Actors[0];

	// 1. プレイヤーを敵の正面/背後に移動させる
	ACombatCharacterBase* Owner = GetCombatCharacter();
	// 正面か背後か判定する
	const FVector ToSource = (Owner->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
	const FVector TargetForward = Owner->GetActorForwardVector().GetSafeNormal2D();
	const float DotProduct = FVector::DotProduct(ToSource, TargetForward);
	const bool bIsFront = DotProduct >= 0.0f;

	// 2. 敵の体幹崩れを解除する

	// 3. 敵に倒れるモーションを再生する(終了は待たない)

	// 4. 同時にプレイヤーにとどめモーションを再生する(終了を待つ)
}
//*/
