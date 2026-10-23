// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CombatDamageExeCalc.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UCombatDamageExeCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

	UCombatDamageExeCalc();
	
public:
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;

private:
	// 直撃時の処理
	void HitDirect()const;
};
