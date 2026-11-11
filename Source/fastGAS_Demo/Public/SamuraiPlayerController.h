// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
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
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Input")
	void SetPlayerInputEnabled(bool bEnabled = false);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	bool bAutoInputDisableOnBeginPlay = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LockAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	float TurnAroundSpeedForLockon = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> GuardAction;

private:
	UPROPERTY()
	TObjectPtr<ASamuraiCharacter> ControlledCharacter;

	void Input_Move(const FInputActionValue& Value);
	void Input_MoveEnd(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Attack(const FInputActionValue& Value);
	void Input_Lock(const FInputActionValue& Value);
	void Input_Dodge(const FInputActionValue& Value);
	void Input_Guard(const FInputActionValue& Value);

	UFUNCTION()
	void OnDodgeTagChanged(FGameplayTag Tag, int32 NewCount);
	FDelegateHandle HandleForOnDodgeTagChanged;
};
