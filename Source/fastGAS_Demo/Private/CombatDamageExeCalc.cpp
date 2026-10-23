// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatDamageExeCalc.h"
#include "AbilitySystemComponent.h"
#include "CombatGameplayTags.h"

UCombatDamageExeCalc::UCombatDamageExeCalc()
{

}

void UCombatDamageExeCalc::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	// 攻撃側(Source)と被弾側(Target)のアクターを取得
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;

	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	if (!SourceActor || !TargetActor)
	{
		return;
	}

	// 互いの向きを判定
	const FVector ToSource = (SourceActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
	const FVector TargetForward = TargetActor->GetActorForwardVector().GetSafeNormal2D();

	const float DotProduct = FVector::DotProduct(ToSource, TargetForward);

	const bool bIsFront = DotProduct > 0.0f;
	UE_LOG(LogTemp, Warning, TEXT("[CombatDamageExeCalc] Dot: %f -> %s"),
		DotProduct, bIsFront ? TEXT("FRONT(正面)") : TEXT("BACK(背後)")
	);

	// 背後から攻撃したとき
	if (!bIsFront)
	{
		// 直撃
		HitDirect();
		return;
	}
	// 正面から攻撃したとき
	else
	{
		const bool bIsParrying = TargetASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Parry);
		const bool bIsGuarding = TargetASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Guard);

		if (bIsParrying)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CombatDamageExeCalc] Hit was Parried"));

		}
		else if (bIsGuarding)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CombatDamageExeCalc] Hit was Guarded"));

		}
		else
		{
			// 直撃
			HitDirect();
		}

		return;
	}
}

void UCombatDamageExeCalc::HitDirect()const
{
	UE_LOG(LogTemp, Warning, TEXT("[CombatDamageExeCalc] Hit Direct"));
}
