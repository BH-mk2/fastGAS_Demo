// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_ApplyCurveMovement.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UANS_ApplyCurveMovement : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FName CurveName = FName("StepSpeed");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SpeedMultiplier = 1.0f;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
