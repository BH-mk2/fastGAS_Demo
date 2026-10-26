// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatCharacterBase.h"
#include "InputActionValue.h"
#include "CombatGameplayAbility.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API APlayerCharacter final : public ACombatCharacterBase
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();

	virtual FVector GetDesiredMoveDirection() const override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaSeconds) override;


	virtual bool GetAttackPoints(FVector& OutStart, FVector& OutEnd)const override;

	/* 移動入力ハンドラ */
	void Move(const FInputActionValue& Value);

	/* 視点操作入力ハンドラ */
	void Look(const FInputActionValue& Value);

	/* 回避入力ハンドラ */
	void Dodge();

	/* 攻撃入力ハンドラ */
	void Attack();

	/* ガード入力ハンドラ */
	void Guard(const FInputActionValue& Value);

	/* ロックオン入力ハンドラ */
	void Lock(const FInputActionValue& Value);

protected:
	/* カメラブーム */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* CameraBoom;

	/* フォローカメラ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* FollowCamera;

	/* 入力マッピングコンテキスト */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputData")
	UInputMappingContext* DefaultMappingContext;

	/* 移動用 InputAction (IA_Move) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputData")
	UInputAction* MoveAction;

	/* 視点用 InputAction (IA_Look) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputData")
	UInputAction* LookAction;

	/* 回避用 InputAction (IA_Dodge) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputData")
	UInputAction* DodgeAction;

	/* 攻撃用 InputAction (IA_Attack) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputData")
	UInputAction* AttackAction;

	/* 防御用 InputAction (IA_Guard) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputData")
	UInputAction* GuardAction;

	/* ロック用 InputAction (IA_Lock) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputData")
	UInputAction* LockAction;

	/* 回避アビリティ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	TSubclassOf<class UCombatGameplayAbility> DodgeAbilityClass;

	/* 攻撃アビリティ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UCombatGameplayAbility> AttackAbilityClass;

	/* 防御アビリティ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UCombatGameplayAbility> GuardAbilityClass;

	/* 忍殺アビリティ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UCombatGameplayAbility> DeathblowAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LockOn")
	float LockOnRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LockOn")
	float LockOnInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	FVector LockOnOffset = FVector(0.0f, 90.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ninsatsu")
	float NinsatsuLength = 50.0f;
private:
	bool IsLock=false;
	TObjectPtr<AActor> LockOnTarget;

	FVector DefaultSocketOffset;

	void UpdateCamera(float DeltaSeconds);
	void LockProcess(AActor* Target);
	void UnLockProcess();

	void OnDodgeTagChanged(FGameplayTag Tag, int32 NewCount);

	void NormalAttackProcess();
	// 戻り値: true=忍殺を行う/false=忍殺は行わない
	bool NinsatsuAttackProcess();
};
