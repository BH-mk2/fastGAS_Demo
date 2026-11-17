// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StageManagementSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageResetSignature);

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UStageManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 現在の復活地点を登録
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void SetRespawnTransform(const FTransform& NewTransform);

	// 登録されている復活地点を取得
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool GetRespawnTransform(FTransform& OutTransform) const;

	// ワールド内の全てのIStageResettableに対してリセットを実行
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ResetStage();

	// デリゲートでリセット通知を受け取りたい場合用
	UPROPERTY(BlueprintAssignable, Category = "Stage")
	FOnStageResetSignature OnStageResetDelegate;

private:
	std::optional<FTransform> CurrentRespawnTransform;
};
