// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "GA_Stagger.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UGA_Stagger : public UCombatGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Stagger();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageBlendOut();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnStaggerTagRemoved();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stagger")
	TSubclassOf<UGameplayEffect> StaggerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stagger")
	TSubclassOf<UGameplayEffect> EffectOnEnd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stagger")
	TObjectPtr<UAnimMontage> StaggerMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stagger")
	FName RecoverSectionName = FName("Recover");

	FActiveGameplayEffectHandle ActivateStaggerEffectHandle;
};
