// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include <Abilities/GameplayAbilityTypes.h>

#include "DeathblowAction.generated.h"

class ACharacter;
class UAnimMontage;
class UGameplayEffect;
class USpringArmComponent;
class UCameraComponent;
class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathblowActionFinished);

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class FASTGAS_DEMO_API UDeathblowAction : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Deathblow")
	FOnDeathblowActionFinished OnFinished;

	UFUNCTION(BlueprintNativeEvent, Category = "Deathblow")
	void Execute(ACharacter* Attacker, ACharacter* Victim);
	virtual void Execute_Implementation(ACharacter* Attacker, ACharacter* Victim);

	virtual UWorld* GetWorld() const override;

protected:
	virtual void FinishAction();

	UPROPERTY(Transient)
	TWeakObjectPtr<ACharacter> CachedAttacker;

	UPROPERTY(Transient)
	TWeakObjectPtr<ACharacter> CachedVictim;

	// 忍殺ダメージ用 GameplayEffect（GE_DeathblowDamage）
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DeathblowDamageEffect;

	void ApplyResultEffect();
	bool bEffectApplied = false;
};

UCLASS()
class FASTGAS_DEMO_API UDeathblowAction_Standard : public UDeathblowAction
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(ACharacter* Attacker, ACharacter* Victim) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackerMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> VictimMontage;

	// 被害者から見た攻撃者の相対位置オフセット（正面距離など）
	UPROPERTY(EditDefaultsOnly, Category = "Positioning")
	float FrontDistance = 120.0f;

	// --- カメラ演出系 ---
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	bool bEnableCameraZoom = true;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (EditCondition = "bEnableCameraZoom"))
	float ZoomArmLength = 220.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (EditCondition = "bEnableCameraZoom"))
	FVector ZoomSocketOffset = FVector(0.0f, 45.0f, 10.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (EditCondition = "bEnableCameraZoom"))
	float ZoomFOV = 75.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (EditCondition = "bEnableCameraZoom"))
	float CameraInterpSpeed = 8.0f;

	// 攻撃を当てたときのカメラ揺れアセット
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	virtual void FinishAction() override;

private:
	void OnAttackerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnImpactEventReceived(const FGameplayEventData* Payload);
	FDelegateHandle ImpactEventHandle;

	void StartCameraZoom(ACharacter* Attacker);
	void UpdateCameraInterp();
	void RestoreCamera();
	FTimerHandle CameraTimerHandle;

	TWeakObjectPtr<USpringArmComponent> CachedSpringArm;
	TWeakObjectPtr<UCameraComponent> CachedCamera;

	float OriginalArmLength = 0.0f;
	FVector OriginalSocketOffset = FVector::ZeroVector;
	float OriginalFOV = 90.0f;
	bool bIsZoomingIn = false;
};

UCLASS()
class FASTGAS_DEMO_API UDeathblowAction_Cinematic : public UDeathblowAction
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(ACharacter* Attacker, ACharacter* Victim) override;

protected:
	virtual void FinishAction() override;

	// 再生するLevelSequenceアセット
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	TObjectPtr<ULevelSequence> DeathblowSequence;

	// シーケンス内のアクターバインディング用タグ
	UPROPERTY(EditDefaultsOnly, Category = "Cinematic|Binding")
	FName AttackerBindingTag = FName("Attacker");

	UPROPERTY(EditDefaultsOnly, Category = "Cinematic|Binding")
	FName VictimBindingTag = FName("Victim");

	// trueの場合、ターゲットの現在位置と向きを原点としてシーケンスを再生する
	UPROPERTY(EditAnywhere, Category = "Cinematic|Transform")
	bool bUseVictimTransformOrigin = true;

	// 終了時に元のカメラへ戻るブレンド時間
	UPROPERTY(EditDefaultsOnly, Category = "Cinematic|Camera")
	float CameraBlendOutSeconds = 0.4f;

	// trueの場合、LS終了時のカメラの向きにプレイヤーの始点を同期する
	UPROPERTY(EditAnywhere, Category = "Cinematic|Camera")
	bool bSyncControlRotation = true;

private:
	void OnImpactEventReceived(const FGameplayEventData* Payload);

	UFUNCTION()
	void OnSequenceFinished();

	void CleanupSequenceActor();

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> SequencePlayer;

	UPROPERTY(Transient)
	TWeakObjectPtr<ALevelSequenceActor> CachedSequenceActor;

	FDelegateHandle ImpactEventHandle;
	FTimerHandle CleanupTimerHandle;
};
