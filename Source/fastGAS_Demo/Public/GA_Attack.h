// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "GA_Attack.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UGA_Attack : public UCombatGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnComboEventReceived(FGameplayEventData payload);

	UFUNCTION()
	void OnEvaluateBufferEventReceived(FGameplayEventData payload);

	// 攻撃用アニメーション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* AttackMontage;

	// アニメーション再生速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	float PlayRate = 1.0f;

	// 最大コンボ数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combo")
	TArray<FName> ComboSectionNames;

private:
	void ProcessNextCombo(UAbilitySystemComponent& ASC);
private:
	int32 ComboIndex = 0;
	bool bComboBuffer = false;
};
