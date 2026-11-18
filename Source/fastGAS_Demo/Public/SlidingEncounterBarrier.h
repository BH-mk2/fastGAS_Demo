// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EncounterBarrier.h"
#include "SlidingEncounterBarrier.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API ASlidingEncounterBarrier : public AEncounterBarrier
{
	GENERATED_BODY()
	
public:

	ASlidingEncounterBarrier();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void UpdateBarrierProgress_Implementation(float Alpha) override;

	/** 見た目専用の柵・門メッシュ（コリジョンを持たせない） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** 封鎖時のメッシュ相対位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier", Meta = (MakeEditWidget = true))
	FVector ClosedLocation = FVector::ZeroVector;

	/** 開放時のメッシュ相対位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier", Meta = (MakeEditWidget = true))
	FVector OpenLocation = FVector(0.0f, 0.0f, -300.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier", Meta = (ClampMin = "0.01"))
	float TransitionDuration = 1.0f;

#if WITH_EDITORONLY_DATA
	/** エディタ上で封鎖状態・開放状態のどちらを表示するか */
	UPROPERTY(EditAnywhere, Category = "Barrier")
	bool bPreviewClosedInEditor = true;
#endif
};
