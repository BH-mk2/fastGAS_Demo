// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "GA_Death.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UGA_Death : public UCombatGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Death();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* DefaultDeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* DeathblowDeathMontage;

	UFUNCTION()
	void OnDeathMontageFinished();
};
