// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FASTGAS_DEMO_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 攻撃の始点と終点を求める関数
	// 戻り値:存在する場合はtrue/ない場合はfalse
	virtual bool GetAttackPoints(FVector& OutStart, FVector& OutEnd)const = 0;

	// 衝突判定用のヒット済みリストをリセット
	virtual void ResetHitActors() = 0;

	// 衝突判定用のヒット済みリストにActorを追加
	// 戻り値:既に登録している場合はfalse/ない場合はtrue
	virtual bool AddHitActor(TWeakObjectPtr<AActor> HitActor) = 0;
};
