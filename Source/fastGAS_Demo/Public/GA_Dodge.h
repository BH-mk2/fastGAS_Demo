// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "GA_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UGA_Dodge : public UCombatGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	UFUNCTION()
	void OnMontageEnded();

	// 前回避用アニメーション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Motion")
	UAnimMontage* ForwardDodgeMontage;

	// 後ろ回避用アニメーション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* BackwardDodgeMontage;

	// アニメーション再生速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	float PlayRate = 1.0f;
};
