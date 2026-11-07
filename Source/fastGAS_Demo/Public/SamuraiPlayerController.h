// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "SamuraiPlayerController.generated.h"

class ASamuraiCharacter;
class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API ASamuraiPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintCallable, Category="Input")
	void SetPlayerInputEnabled(bool bEnabled = false);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

private:
	UPROPERTY()
	TObjectPtr<ASamuraiCharacter> ControlledCharacter;

	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
};
