// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatCharacterBase.h"
#include "SamuraiCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmedStateChangedSignature, bool, bNewIsArmed);

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API ASamuraiCharacter : public ACombatCharacterBase
{
	GENERATED_BODY()

public:
	ASamuraiCharacter();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void UnsheatheWeapon();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SheatheWeapon();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void OnUnsheatheEnded();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnSheatheEnded();

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnArmedStateChangedSignature OnArmedStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaSeconds) override;

	void SetArmedState(bool bNewState);

protected:
	/* カメラブーム */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/* フォローカメラ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	/* 抜刀モーション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* UnsheatheMotion;

	/* 納刀モーション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	UAnimMontage* SheatheMotion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	FName SocketForSheathe = TEXT("Socket_Sheath");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion")
	FName SocketForHand = TEXT("WeaponSocket");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsArmed = false;

private:
	void AttachWeaponToHand();

	void AttachWeaponToSheath();

	void PlayWeaponMontage(UAnimMontage* MontageToPlay, bool bIsNewArmedState);

	UFUNCTION()
	void OnWeaponMontageEnded(FName NotifyName);

	FOnMontageEnded WeaponMontageEndedDelegate;
	bool bPendingArmedState = false;
};
