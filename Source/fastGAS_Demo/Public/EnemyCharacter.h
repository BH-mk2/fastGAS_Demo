// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatCharacterBase.h"
#include "CombatGameplayAbility.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API AEnemyCharacter : public ACombatCharacterBase
{
	GENERATED_BODY()
	
	virtual FVector GetDesiredMoveDirection() const override { return FVector::ZeroVector; }
public:
	AEnemyCharacter();

	UFUNCTION(BlueprintCallable)
	void ExecuteAttack();

	UFUNCTION(BlueprintCallable)
	void ExecuteGuard();

	UFUNCTION(BlueprintCallable)
	void ExecuteGuardEnd();
protected:
	void BeginPlay() override;

	bool GetAttackPoints(FVector& OutStart, FVector& OutEnd)const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	TSubclassOf<class UCombatGameplayAbility> AttackAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	TSubclassOf<class UCombatGameplayAbility> GuardAbilityClass;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackCancellableDelegate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEndDelegate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuardEndDelegate);

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnAttackCancellableDelegate OnAttackCancellable;

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnAttackEndDelegate OnAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnGuardEndDelegate OnGuardEnd;

private:
	void OnAttackCancellableInner(FGameplayTag Tag, int32 count);
	void OnAttackEndInner(FGameplayTag Tag, int32 count);
	void OnGuardEndInner(FGameplayTag Tag, int32 count);
};
