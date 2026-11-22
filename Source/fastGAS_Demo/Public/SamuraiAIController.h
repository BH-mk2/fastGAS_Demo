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
	virtual void SetFocus(AActor* NewFocus, EAIFocusPriority::Type InPriority = EAIFocusPriority::Gameplay) override;

	UFUNCTION(BlueprintCallable)
	virtual void SetCombatFocus(AActor* Target);

	virtual void ClearFocus(EAIFocusPriority::Type InPriority) override;
	UFUNCTION(BlueprintCallable)
	virtual void ClearCombatFocus();

	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;
};
