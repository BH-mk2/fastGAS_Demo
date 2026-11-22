// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatCharacterBase.h"
#include "SamuraiCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UDeathblowAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmedStateChangedSignature, bool, bNewIsArmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuardedStateChangedSignature, bool, bNewIsGuarded);

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API ASamuraiCharacter : public ACombatCharacterBase
{
	GENERATED_BODY()

public:
	ASamuraiCharacter();

	void SetDesiredMoveDirection(const FVector& Direction);
	virtual FVector GetDesiredMoveDirection() const override;

	UFUNCTION(BlueprintCallable, Category="Combat")
	void UnsheatheWeapon();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SheatheWeapon();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void OnUnsheatheEnded();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnSheatheEnded();

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnArmedStateChangedSignature OnArmedStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnGuardedStateChangedSignature OnGuardedStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecAttack();
	
	void ExecDodge();
	void HoldSprintInput();
	void ReleaseSprintInput();

	// void RequestMove(const FVector& WorldDirection, float Scale);
	// オーバーライドするが移動は親に任せる。回転だけ独自の制御にする
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;
	virtual bool IsMoveInputIgnored() const override;
	void SetLockOnTarget(TWeakObjectPtr<AActor> Target);
	void LockOff();
	TWeakObjectPtr<AActor> GetLockedOnTarget();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartGuard();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopGuard();

	UDeathblowAction* GetDeathblowAction() { return DeathblowAction; }

	void RespawnAtCheckpoint();

	TEnumAsByte<ECollisionChannel> GetWeaponTraceChannel()const { return WeaponTraceChannel; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual bool GetAttackPoints(FVector& OutStart, FVector& OutEnd)const override;

	void SetArmedState(bool bNewState);

protected:
	/* 抜刀モーション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* UnsheatheMotion;

	/* 納刀モーション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* SheatheMotion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	FName SocketForSheathe = TEXT("Socket_Sheath");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	FName SocketForHand = TEXT("WeaponSocket");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsArmed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float DeathblowSearchRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Deathblow")
	TObjectPtr<class UDeathblowAction> DeathblowAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	TEnumAsByte<ECollisionChannel> WeaponTraceChannel;

private:
	void AttachWeaponToHand();

	void AttachWeaponToSheath();

	void PlayWeaponMontage(UAnimMontage* MontageToPlay, bool bIsNewArmedState);

	UFUNCTION()
	void OnWeaponMontageEnded(FName NotifyName);

	FOnMontageEnded WeaponMontageEndedDelegate;
	bool bPendingArmedState = false;

	TWeakObjectPtr<AActor> LockedOnTarget;

	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);

	FVector DesiredMoveDirection = FVector::ZeroVector;

	UFUNCTION()
	void OnGuardTagChanged(FGameplayTag Tag, int32 NewCount);

	TObjectPtr<AActor> FindDeathblowTarget();

	void ExecDeathblow(TObjectPtr<AActor> Target);

	void UpdateRotate(float DeltaTime);
};
