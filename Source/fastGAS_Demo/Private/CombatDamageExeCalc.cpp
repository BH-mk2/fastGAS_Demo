// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatDamageExeCalc.h"
#include "AbilitySystemComponent.h"
#include "CombatGameplayTags.h"
#include "CombatAttributeSet.h"

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

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

	const FHitResult* HitResult = ContextHandle.GetHitResult();

	FVector ImpactLocation = FVector::ZeroVector;
	if (HitResult)
	{
		ImpactLocation = HitResult->ImpactPoint;
	}
	else
	{
		if (TargetASC && TargetASC->GetAvatarActor())
		{
			ImpactLocation = TargetASC->GetAvatarActor()->GetActorLocation();
		}
	}

	// 背後から攻撃したとき
	if (!bIsFront)
	{
		// 直撃
		HitDirect(ExecutionParams, OutExecutionOutput, ImpactLocation);
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

			FGameplayCueParameters CueParams;
			CueParams.Location = ImpactLocation;
			TargetASC->ExecuteGameplayCue(CombatGameplayTags::CGTAG_GAMEPLAYCUE_Hit_Parry, CueParams);
		}
		else if (bIsGuarding)
		{
			// 防御側の体幹を増やす
			UE_LOG(LogTemp, Warning, TEXT("[CombatDamageExeCalc] Hit was Guarded"));

			const float DamageHealth = Spec.GetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Health);
			const float DamagePosture = Spec.GetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture);
			const float ApplyDamagePosture = DamagePosture * GuardDamagePostureRate;

			auto Attributes = TargetASC->GetSet<UCombatAttributeSet>();
			const float CurrentPosture = Attributes->GetPosture();

			OutExecutionOutput.AddOutputModifier(
				FGameplayModifierEvaluatedData(
					UCombatAttributeSet::GetPostureAttribute(),
					EGameplayModOp::Additive,
					ApplyDamagePosture
				)
			);

			{
				FGameplayCueParameters CueParams;
				CueParams.Location = ImpactLocation;
				TargetASC->ExecuteGameplayCue(CombatGameplayTags::CGTAG_GAMEPLAYCUE_Hit_Guard, CueParams);
			}

			// 体幹が最大値になっていたら体幹崩れの処理をする
			if (!TargetASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered))
			{
				if ((CurrentPosture + ApplyDamagePosture) >= Attributes->GetMaxPosture())
				{
					TargetASC->AddLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered);
				}
			}
		}
		else
		{
			// 直撃
			HitDirect(ExecutionParams, OutExecutionOutput, ImpactLocation);
		}

		return;
	}
}

void UCombatDamageExeCalc::HitDirect(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput,
	const FVector& HitLocation
)const
{
	UE_LOG(LogTemp, Warning, TEXT("[CombatDamageExeCalc] Hit Direct"));

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const float DamageHealth = Spec.GetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Health);
	const float DamagePosture= Spec.GetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture);

	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!TargetASC)
		return;

	auto Attributes = TargetASC->GetSet<UCombatAttributeSet>();
	const float CurrentPosture = Attributes->GetPosture();

	// 防御側の体力を減らす
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UCombatAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			DamageHealth
		)
	);
	// 防御側の体幹を増やす
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UCombatAttributeSet::GetPostureAttribute(),
			EGameplayModOp::Additive,
			DamagePosture
		)
	);

	if (AActor* TargetActor = TargetASC->GetAvatarActor())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitLocation;
		TargetASC->ExecuteGameplayCue(CombatGameplayTags::CGTAG_GAMEPLAYCUE_Hit_Direct, CueParams);
	}

	// 体幹が最大値になっていたら体幹崩れの処理をする
	if (!TargetASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered))
	{
		if ((CurrentPosture + DamagePosture) >= Attributes->GetMaxPosture())
		{
			TargetASC->AddLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered);
		}
	}
}
