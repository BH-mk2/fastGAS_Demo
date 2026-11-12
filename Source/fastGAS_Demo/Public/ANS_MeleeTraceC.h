// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayEffect.h"
#include "ANS_MeleeTraceC.generated.h"

class ICombatInterface;

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UANS_MeleeTraceC : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	// virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Collision", meta = (ClampMin = "0"))
	float Radius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	TSubclassOf<UGameplayEffect> DirectHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> HitBlockedEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> ParryEffect;

	// TODO: 後で消す。（キャラのパラメータで行う為）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	float DamageHealthMagnitude = -20.0f;

	// TODO: 後で消す。（キャラのパラメータで行う為）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	float DamagePostureMagnitude = 20.0f;
private:
	void HitProcess(AActor* Owner, ICombatInterface* CombatInterface, TArray<FHitResult>& Hits);
	enum class EHitKind
	{
		Direct,
		Guard,
		Parry,
	};
	EHitKind CheckHitResult(AActor& Src, UAbilitySystemComponent& SrcASC, AActor & Target, UAbilitySystemComponent& TargetASC);

	class FInnerProcessor
	{
	public:
		explicit FInnerProcessor(UANS_MeleeTraceC& Context, AActor& Src, UAbilitySystemComponent& SrcASC, AActor& Target, UAbilitySystemComponent& TargetASC, FHitResult& HitResult);
		// 自分が相手に直撃させたときの処理
		void ProcessDirect();
		// 自分が相手にガードさせたときの処理
		void ProcessGuard();
		// 自分が相手に弾きさせたときの処理
		void ProcessParry();
	private:
		UANS_MeleeTraceC& Context;
		AActor& Src;
		UAbilitySystemComponent& SrcASC;
		AActor& Target;
		UAbilitySystemComponent& TargetASC;
		FHitResult& HitResult;
	};
};
