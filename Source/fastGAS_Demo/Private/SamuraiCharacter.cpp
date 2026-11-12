// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayMontageCallbackProxy.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Kismet/KismetSystemLibrary.h"

#include "CombatGameplayAbility.h"
#include "WeaponBase.h"
#include "CombatAbilityInputID.h"
#include "CombatGameplayTags.h"

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

FVector ASamuraiCharacter::GetDesiredMoveDirection() const
{
	return DesiredMoveDirection;
}

void ASamuraiCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Weapon)
	{
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketForSheathe);
	}

	if (!AttackAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbilityClass, 1));

	if (!DodgeAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DodgeAbilityClass, 1));

	if (!GuardAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GuardAbilityClass, 1, static_cast<int32>(ECombatAbilityInputID::Guard)));

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Guard)
		.AddUObject(this, &ASamuraiCharacter::OnGuardTagChanged);

	if (!StaggerAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StaggerAbilityClass, 1));
}
void ASamuraiCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASamuraiCharacter::Tick(float DeltaSeconds)
{

}

bool ASamuraiCharacter::GetAttackPoints(FVector& OutStart, FVector& OutEnd)const
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
	MontageProxy->OnBlendOut.AddDynamic(this, &ASamuraiCharacter::OnWeaponMontageEnded);
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

void ASamuraiCharacter::ExecAttack()
{
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

void ASamuraiCharacter::ExecDodge()
{
	AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbilityClass);
}

void ASamuraiCharacter::RequestMove(const FVector& WorldDirection, float Scale)
{
	if (WorldDirection.IsNearlyZero())
	{
		DesiredMoveDirection = FVector::ZeroVector;
		return;
	}

	DesiredMoveDirection = WorldDirection;

	const bool bIsMovementDisabled = AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Movement_Disable);
	const bool bCanSmoothRotate = AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Rotation_SmoothWindow);
	const bool bCanInstantRotate = AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Rotation_InstantWindow);

	const float TurnAroundSpeed = 10.0f;

	// ① 通常時：足が動かせる状態
	if (!bIsMovementDisabled)
	{
		// 移動入力を流す
		if (auto Target = LockedOnTarget.Get())
		{
			// ロックオン中は方向入力があった場合、ターゲットの方向に回転する
			if (!WorldDirection.IsNearlyZero())
			{
				const FRotator CurrentRot = GetActorRotation();
				const FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				const FRotator TargetRot = FRotator(0.0f, DirectionToTarget.Rotation().Yaw, 0.0f);
				const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), TurnAroundSpeed);
				SetActorRotation(NewRot);
			}
		}
		AddMovementInput(WorldDirection, Scale);
	}
	// ② 攻撃中など：足が止まっている状態
	else
	{
		// 足は止まっているが、初動の向き調整ウィンドウが開いている時だけ手動で回す
		// 即座に回転して良い場合
		if (bCanInstantRotate)
		{
			SetActorRotation(FRotator(0.0f, WorldDirection.Rotation().Yaw, 0.0f));
		}
		else if(bCanSmoothRotate)
		{
			// 滑らかに向きを変える
			const FRotator CurrentRot = GetActorRotation();
			const FRotator TargetRot = FRotator(0.0f, WorldDirection.Rotation().Yaw, 0.0f);
			const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), TurnAroundSpeed);
			SetActorRotation(NewRot);
		}
	}
}

TWeakObjectPtr<AActor> ASamuraiCharacter::SearchTarget()
{
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	constexpr bool bTraceComplex = false;
	const TArray<AActor*> ActorsToIgnore = { this };
	TArray<FHitResult> OutHits;

	// Sweepをカメラの前方に飛ばして一番近いターゲットにロックオンする
	// 1. 球を飛ばす
	const FVector Start = GetActorLocation();
	const FVector End = Start;
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, LockOnRadius, ObjectTypes, bTraceComplex, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);

	// 2. カメラ前方にいるものだけにフィルターする
	const FVector ViewForward = GetViewRotation().Vector();

	double BestLength = -1.0f;
	AActor* BestTarget = nullptr;

	for (FHitResult& Enemy : OutHits)
	{
		const auto TargetActor = Enemy.GetActor();
		if (!TargetActor)continue;
		const FVector ForwardVec = ViewForward;
		const FVector ToTargetVec = TargetActor->GetActorLocation() - GetActorLocation();
		const double ToTargetLength = ToTargetVec.Length();
		const FVector ToTargetNormal = ToTargetVec.GetSafeNormal2D();
		const float DotProduct = FVector::DotProduct(ForwardVec, ToTargetNormal);

		// 正面ではないものは無視
		if (DotProduct < 0.0f)
			continue;

		if (ToTargetLength > BestLength)
		{
			BestLength = ToTargetLength;
			BestTarget = TargetActor;
		}
	}

	return BestTarget;
}

void ASamuraiCharacter::ToggleLockOnTarget()
{
	// 既にロックオンしている場合、外して終了
	if (LockedOnTarget.IsValid())
	{
		LockedOnTarget->OnDestroyed.RemoveDynamic(this, &ASamuraiCharacter::OnTargetDestroyed);
		LockOff();
		return;
	}

	// ターゲットを探索して、存在したらロックオンする
	LockedOnTarget = SearchTarget();
	if (!LockedOnTarget.IsValid())
	{
		// 存在しないので終了
		LockedOnTarget = nullptr;
		return;
	}

	GetCharacterMovement()->bOrientRotationToMovement = false;
	LockedOnTarget->OnDestroyed.AddDynamic(this, &ASamuraiCharacter::OnTargetDestroyed);

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LockedOnTarget.Get()))
	{
		TargetASC->AddLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Combat_LockedOn);
	}
}
void ASamuraiCharacter::LockOff()
{
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LockedOnTarget.Get()))
	{
		TargetASC->RemoveLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Combat_LockedOn);
	}
	LockedOnTarget = nullptr;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ASamuraiCharacter::OnTargetDestroyed(AActor* DestroyedActor)
{
	LockOff();
}

void ASamuraiCharacter::OnGuardTagChanged(FGameplayTag Tag, int32 NewCount)
{
	const bool bIsGuarded = NewCount > 0;

	OnGuardedStateChanged.Broadcast(bIsGuarded);
}

TWeakObjectPtr<AActor> ASamuraiCharacter::GetLockedOnTarget()
{
	return LockedOnTarget;
}

void ASamuraiCharacter::StartGuard()
{
	AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(ECombatAbilityInputID::Guard));
}

void ASamuraiCharacter::StopGuard()
{
	AbilitySystemComponent->AbilityLocalInputReleased(static_cast<int32>(ECombatAbilityInputID::Guard));
}
