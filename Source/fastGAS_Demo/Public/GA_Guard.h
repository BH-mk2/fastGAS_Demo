// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "GA_Guard.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UGA_Guard : public UCombatGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Guard();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnGuardEndEventReceived(FGameplayEventData payload);

	// 防御用アニメーション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* GuardMontage;

	// アニメーション再生速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	FName EndSectionName = FName("End");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	FName HitSectionName = FName("Hit");

	// 弾き有効時間
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	float ParrySec = 0.25f;

	// 弾きを成功させたときのGameplayEffect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> ParryEffect;

private:
	bool IsReleasedParry = false;
	
	void ReleaseParry();
	FTimerHandle ParryTimerHandle;

	UFUNCTION()
	void OnInputReleased(float TImeHeld);

	UFUNCTION()
	void OnHitBlockReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnParrySuccessReceived(FGameplayEventData Payload);
};
