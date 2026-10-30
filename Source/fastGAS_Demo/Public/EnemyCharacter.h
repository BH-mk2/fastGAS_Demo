// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatCharacterBase.h"
#include "CombatGameplayAbility.h"
#include "Components/WidgetComponent.h"
#include "EnemyCharacter.generated.h"

class UWidgetComponent;

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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockChangedDelegate, bool, Value);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaggerChangedDelegate, bool, Value);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevivalDelegate);

	UPROPERTY(BlueprintAssignable, Category="Combat|Event")
	FOnAttackCancellableDelegate OnAttackCancellable;

	UPROPERTY(BlueprintAssignable, Category="Combat|Event")
	FOnAttackEndDelegate OnAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnGuardEndDelegate OnGuardEnd;

	UPROPERTY(BlueprintAssignable, Category="Combat|Event")
	FOnLockChangedDelegate OnLockChanged;

	UPROPERTY(BlueprintAssignable, Category="Combat|Event")
	FOnStaggerChangedDelegate OnStaggerChanged;

	UPROPERTY(BlueprintAssignable, Category="Combat|Event")
	FOnDeathDelegate OnDeath;

	UPROPERTY(BlueprintAssignable, Category="Combat|Event")
	FOnRevivalDelegate OnRevival;

	// 死亡用アニメーション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* DeathMontage;
	// 体幹崩れ用アニメーション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* StaggerMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget")
	TObjectPtr<UWidgetComponent> WidgetBars;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> WidgetTargetMarker;

private:
	void OnAttackCancellableInner(FGameplayTag Tag, int32 count);
	void OnAttackEndInner(FGameplayTag Tag, int32 count);
	void OnGuardEndInner(FGameplayTag Tag, int32 count);
	void OnLockChangedInner(FGameplayTag Tag, int32 count);
	void OnStaggerChangedInner(FGameplayTag Tag, int32 count);

	void OnHealthChangedInner(const FOnAttributeChangeData& Data);
};
