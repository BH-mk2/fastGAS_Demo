// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SamuraiAIController.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API ASamuraiAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ASamuraiAIController();
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintCallable)
	virtual void SetCombatFocus(AActor* Target);

	UFUNCTION(BlueprintCallable)
	virtual void ClearCombatFocus(AActor* ForgottenActor);

	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;
};
