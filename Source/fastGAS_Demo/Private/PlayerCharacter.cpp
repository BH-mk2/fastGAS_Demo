// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubSystems.h"

APlayerCharacter::APlayerCharacter()
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

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced InputのMapping Contextを登録
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if(!PlayerController)
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!Subsystem)
		return;

	if (!DefaultMappingContext)
		return;

	Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
		return;

	if (!MoveAction)
		return;

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

	if (!LookAction)
		return;

	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

	if (!DodgeAction)
		return;

	EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::Dodge);

	if (!DodgeAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DodgeAbilityClass, 1));
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	LLM_SCOPE_BYNAME(TEXT("PlayerCharacterInitialize"));

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (!Controller)
		return;

	// カメラ(コントローラ)の水平方向(Yaw)を取得
	const FRotator Rotation = Controller->GetControlRotation();

	const FRotator YawRotation(0.0, Rotation.Yaw, 0.0);

	// 前方および右方向ベクトルを計算
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 移動入力を適用
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (!Controller)
		return;

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacter::Dodge()
{
	if (!AbilitySystemComponent)
		return;

	if (!DodgeAbilityClass)
		return;

	AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbilityClass);
}
