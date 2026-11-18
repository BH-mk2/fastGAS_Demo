// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterBarrierInterface.h"
#include "EncounterBarrier.generated.h"

class UBoxComponent;

UCLASS()
class FASTGAS_DEMO_API AEncounterBarrier : public AActor, public IEncounterBarrierInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEncounterBarrier();

	virtual void OnSetBarrierActive_Implementation(bool bActive, bool bImmediate) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Barrier")
	void PlayBarrierTransition(bool bClosing);

	UFUNCTION(BlueprintImplementableEvent, Category = "Barrier")
	void StopBarrierTransition(bool bClosedState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Barrier")
	void UpdateBarrierProgress(float Alpha);
	virtual void UpdateBarrierProgress_Implementation(float Alpha) {}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BlockerBox;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
