// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatInterface.h"
#include "AbilitySystemComponent.h"
#include "CombatCharacterBase.generated.h"

class UCombatAttributeSet;
class AWeaponBase;

UCLASS()
class FASTGAS_DEMO_API ACombatCharacterBase : public ACharacter, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatCharacterBase();

	virtual FVector GetDesiredMoveDirection() const {ensureAlways(false);return FVector::ZeroVector;}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UCombatAttributeSet> AttributeSet;

	// 初期ステータスを設定するGEのクラス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	TSubclassOf<class UGameplayEffect> DefaultAttricutesEffect;

	// 武器クラス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<class AWeaponBase> WeaponClass;

	// 生成後の武器
	TObjectPtr<AWeaponBase> Weapon;

	// 武器をアタッチするソケット名
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName WeaponAttachToSocketName;

	// 初期ステータスを設定する関数
	virtual void InitializeDefaultAttributes();
};
