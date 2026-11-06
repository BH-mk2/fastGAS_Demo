// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayMontageCallbackProxy.h"

#include "WeaponBase.h"

ASamuraiCharacter::ASamuraiCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// コントローラの回転にキャラ本体を追従させない(カメラだけ回す)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// キャラクタを移動入力の方向へ滑らかに向かせる
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// スプリングアームの生成と設定
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true; // コントローラの回転に追従

	// カメラの生成
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // アームに対して固定
}

void ASamuraiCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Weapon)
	{
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketForSheathe);
	}
}
void ASamuraiCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASamuraiCharacter::Tick(float DeltaSeconds)
{

}

void ASamuraiCharacter::UnsheatheWeapon()
{
	if (bIsArmed) return;

	PlayWeaponMontage(UnsheatheMotion, true);
}
void ASamuraiCharacter::SheatheWeapon()
{
	if (!bIsArmed) return;

	PlayWeaponMontage(SheatheMotion, false);
}

void ASamuraiCharacter::OnUnsheatheEnded()
{
	AttachWeaponToHand();
}

void ASamuraiCharacter::OnSheatheEnded()
{
	AttachWeaponToSheath();
}

void ASamuraiCharacter::SetArmedState(bool bNewState)
{
	if (bIsArmed == bNewState)
		return;

	bIsArmed = bNewState;

	OnArmedStateChanged.Broadcast(bIsArmed);
}

void ASamuraiCharacter::AttachWeaponToHand()
{
	if (Weapon && GetMesh())
	{
		Weapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			SocketForHand
		);
	}
}

void ASamuraiCharacter::AttachWeaponToSheath()
{
	if (Weapon && GetMesh())
	{
		Weapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			SocketForSheathe
		);
	}
}

void ASamuraiCharacter::PlayWeaponMontage(UAnimMontage* MontageToPlay, bool bIsNewArmedState)
{
	if (!MontageToPlay)
		return;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	bPendingArmedState = bIsNewArmedState;

	UPlayMontageCallbackProxy* MontageProxy = UPlayMontageCallbackProxy::CreateProxyObjectForPlayMontage(
		GetMesh(),
		MontageToPlay,
		1.0f // 再生レート
	);

	if (!MontageProxy)
	{
		// 生成失敗時（スロット不一致や未ロード時）は移動復帰
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		}
		return;
	}

	MontageProxy->OnCompleted.AddDynamic(this, &ASamuraiCharacter::OnWeaponMontageEnded);
	MontageProxy->OnInterrupted.AddDynamic(this, &ASamuraiCharacter::OnWeaponMontageEnded);
}

void ASamuraiCharacter::OnWeaponMontageEnded(FName NotifyName)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	SetArmedState(bPendingArmedState);

	WeaponMontageEndedDelegate.Unbind();
}
