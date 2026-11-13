// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "GA_DirectHitReact.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UGA_DirectHitReact : public UCombatGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_DirectHitReact();

	virtual	void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> FrontReactMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> BackReactMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> LeftReactMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> RightReactMontage;

	UFUNCTION()
	void OnMontageEnded();
};
