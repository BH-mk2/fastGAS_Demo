// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubSystems.h"
#include "WeaponBase.h"
#include "CombatGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "Kismet/KismetSystemLibrary.h"

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

FVector APlayerCharacter::GetDesiredMoveDirection()const
{
	FVector Ret = FVector::ZeroVector;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return Ret;

	UEnhancedPlayerInput* EPI = Cast<UEnhancedPlayerInput>(PC->PlayerInput);
	if (!EPI)
		return Ret;

	const FVector2D MoveInput = EPI->GetActionValue(MoveAction).Get<FVector2D>();
	if (MoveInput.IsNearlyZero())
		return Ret;

	// カメラの向きを取得
	const FRotator ControlRot = PC->GetControlRotation();
	const FRotator YawRot(0.0f, ControlRot.Yaw, 0.0f);

	// カメラの正面と左の向きを算出
	const FRotationMatrix RotationMatrix(YawRot);
	const FVector ForwardDir = RotationMatrix.GetUnitAxis(EAxis::X);
	const FVector RightDir = RotationMatrix.GetUnitAxis(EAxis::Y);

	// カメラの向きに移動入力を足し合わせた向きを算出
	Ret = ForwardDir * MoveInput.Y // 前後
		+ RightDir * MoveInput.X; // 左右


	Ret = Ret.GetSafeNormal();

	return Ret;
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

	if (!AttackAction)
		return;

	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack);

	if (!GuardAction)
		return;

	EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Guard);

	if (!LockAction)
		return;

	EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Lock);

	if (!DodgeAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DodgeAbilityClass, 1));

	if (!AttackAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbilityClass, 1));

	if (!GuardAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GuardAbilityClass, 1));
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	// TODO: ロックオン対象へのカメラ追従
	if (IsLock)
	{
		if (!LockOnTarget)
		{
			IsLock = false;
			GetCharacterMovement()->bOrientRotationToMovement = true;
			bUseControllerRotationYaw = false;
			CameraBoom->SocketOffset = FVector();
		}
		else
		{
			const FVector CenterLocation = GetActorLocation() + (LockOnTarget->GetActorLocation() - GetActorLocation())/2.0f;
			FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CenterLocation);
			const auto& ControlRotation = GetControlRotation();
			FRotator NewRotation = UKismetMathLibrary::RInterpTo(ControlRotation, LookAt, DeltaSeconds, LockOnInterpSpeed);
			NewRotation = FRotator(ControlRotation.Pitch, NewRotation.Yaw, ControlRotation.Roll);

			
			Controller->SetControlRotation(NewRotation);
		}
	}
}

bool APlayerCharacter::GetAttackPoints(FVector& OutStart, FVector& OutEnd)const
{
	if (!Weapon)
		return false;
	auto WeaponMesh = Weapon->GetWeaponMesh();
	if (!WeaponMesh)
		return false;

	OutStart = WeaponMesh->GetSocketLocation("WeaponStart");
	OutEnd = WeaponMesh->GetSocketLocation("WeaponEnd");

	return true;
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

	if (AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action))
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Combat_Cancellable_Dodge))
		{
			UKismetSystemLibrary::PrintString(this, TEXT("[APlayerCharacter] Block Dodge"));
			return;
		}
	}

	const bool Result = AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbilityClass);
	UE_LOG(LogTemp, Warning, TEXT("TryDodgeResult: %d"), Result);
}

void APlayerCharacter::Attack()
{
	if (!AbilitySystemComponent)
		return;

	if (!AttackAbilityClass)
		return;

	if (AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Attack))
	{
		// コンボ入力
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CombatGameplayTags::CGTAG_EVENT_Input_Attack, FGameplayEventData{});
	}
	else
	{
		// 攻撃(ひと振り目)
		AbilitySystemComponent->TryActivateAbilityByClass(AttackAbilityClass);
	}

}

void APlayerCharacter::Guard(const FInputActionValue& Value)
{
	const bool Pressed = Value.Get<bool>();

	if (!AbilitySystemComponent)
		return;

	if (!GuardAbilityClass)
		return;

	// 押したとき
	if (Pressed)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(GuardAbilityClass);
	}
	else
	{
		// 離したとき
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CombatGameplayTags::CGTAG_EVENT_Input_GuardEnd, FGameplayEventData{});
	}
}

void APlayerCharacter::Lock(const FInputActionValue& Value)
{
	const bool Pressed = Value.Get<bool>();

	UE_LOG(LogTemp, Warning, TEXT("[APlayerCharacter] Lock Value:%s"), Pressed ? TEXT("true") : TEXT("false"));

	// ボタンを押したときだけロック状態を切り替える
	if (!Pressed)
		return;

	if (IsLock)
	{
		// 既にロックオンしている場合
		LockOnTarget = nullptr;
		IsLock = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		CameraBoom->SocketOffset = FVector();
		return;
	}

	// これからロックオンする場合
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	constexpr bool bTraceComplex = false;
	const TArray<AActor*> ActorsToIgnore = { this };
	TArray<FHitResult> OutHits;

	// Sweepをカメラの前方に飛ばして一番近いターゲットにロックオンする
	// 1. 球を飛ばす
	const FVector Start = GetActorLocation();
	const FVector End = Start;
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, LockOnRadius, ObjectTypes, bTraceComplex, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);

	// 2. 前方にいるものだけにフィルターする

	float MostNearFrontDot = -1.0f;
	AActor* MostTarget = nullptr;

	for(const FHitResult& Hit : OutHits)
	{
		const auto TargetActor = Hit.GetActor();
		const FVector ForwardVec = GetActorForwardVector();
		const FVector ToTargetVec = TargetActor->GetActorLocation() - GetActorLocation();
		const FVector ToTarget = ToTargetVec.GetSafeNormal2D();
		const float DotProduct = FVector::DotProduct(ForwardVec, ToTarget);

		// 正面ではないものは無視
		if (DotProduct < 0.0f)
			continue;

		if (DotProduct > MostNearFrontDot)
		{
			MostNearFrontDot = DotProduct;
			MostTarget = TargetActor;
			continue;
		}
	}

	// 3. 一番正面かつ近いものにロックオンする
	if (!MostTarget)
		return;

	IsLock = true;
	LockOnTarget = MostTarget;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
	CameraBoom->SocketOffset = LockOnOffset;
}
