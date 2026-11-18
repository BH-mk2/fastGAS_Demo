// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayMontageCallbackProxy.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Kismet/KismetSystemLibrary.h"

#include "CombatGameplayAbility.h"
#include "WeaponBase.h"
#include "CombatAbilityInputID.h"
#include "CombatGameplayTags.h"
#include "StageManagementSubsystem.h"
#include "CombatAttributeSet.h"

ASamuraiCharacter::ASamuraiCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

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

void ASamuraiCharacter::RespawnAtCheckpoint()
{
	UStageManagementSubsystem* StageSubsystem = GetWorld()->GetSubsystem<UStageManagementSubsystem>();
	if (!StageSubsystem)
	{
		return;
	}

	// 復活地点へワープ
	FTransform RespawnTransform;
	if (StageSubsystem->GetRespawnTransform(RespawnTransform))
	{
		SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		if (AController* PC = GetController())
		{
			PC->SetControlRotation(RespawnTransform.GetRotation().Rotator());
		}
	}

	// 死亡状態の解除
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Dead);
		ASC->CancelAllAbilities();
	}
	SetActorEnableCollision(true);
	StopAnimMontage();
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
		{
			AnimInst->Montage_Stop(0.0f);
		}
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (DefaultAttributesEffect)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		ASC->ApplyGameplayEffectToSelf(DefaultAttributesEffect.GetDefaultObject(), 1.0f, Context);
	}
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->SetMovementMode(MOVE_Walking);
		CMC->MaxWalkSpeed = AttributeSet->GetMoveSpeed();
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);
	}

	StageSubsystem->ResetStage();
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

	if (!DeathblowAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DeathblowAbilityClass, 1));

	if (!DodgeAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DodgeAbilityClass, 1));

	if (!SprintAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SprintAbilityClass, 1));

	if (!GuardAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GuardAbilityClass, 1, static_cast<int32>(ECombatAbilityInputID::Guard)));

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Guard)
		.AddUObject(this, &ASamuraiCharacter::OnGuardTagChanged);

	if (!StaggerAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StaggerAbilityClass, 1));

	if (!DirectHitReactAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DirectHitReactAbilityClass, 1));

	if (!DeathAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DeathAbilityClass, 1));
}
void ASamuraiCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(Weapon))
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}

	Super::EndPlay(EndPlayReason);
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
	const bool bIsAttacking = AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Attack);
	const bool bCancellableAttack = AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Combat_Cancellable_Attack);

	TObjectPtr<AActor> DeathblowTarget = FindDeathblowTarget();

	if (bIsAttacking)
	{
		// 忍殺にキャンセルさせる
		if (bCancellableAttack && DeathblowTarget)
		{
			ExecDeathblow(DeathblowTarget);
			return;
		}
		// コンボ入力
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CombatGameplayTags::CGTAG_EVENT_Input_Attack, FGameplayEventData{});
		return;
	}
	else
	{
		if (DeathblowTarget)
		{
			ExecDeathblow(DeathblowTarget);
			return;
		}

		// 攻撃(ひと振り目)
		AbilitySystemComponent->TryActivateAbilityByClass(AttackAbilityClass);

		return;
	}
}

void ASamuraiCharacter::ExecDodge()
{
	// AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbilityClass);
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(DodgeAbilityTag));
}

void ASamuraiCharacter::HoldSprintInput()
{
  	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(SprintAbilityTag));
}

void ASamuraiCharacter::ReleaseSprintInput()
{
	FGameplayTagContainer SprintTags(SprintAbilityTag);
	AbilitySystemComponent->CancelAbilities(&SprintTags);
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
	const bool bIsSprinting = AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Sprint);

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
				// ただし、走り中は入力した方向を向く
				if (bIsSprinting)
				{
					const FRotator CurrentRot = GetActorRotation();
					const FRotator TargetRot = FRotator(0.0f, WorldDirection.Rotation().Yaw, 0.0f);
					const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), TurnAroundSpeed);
					SetActorRotation(NewRot);
				}
				else
				{
					const FRotator CurrentRot = GetActorRotation();
					const FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
					const FRotator TargetRot = FRotator(0.0f, DirectionToTarget.Rotation().Yaw, 0.0f);
					const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), TurnAroundSpeed);
					SetActorRotation(NewRot);
				}
			}
		}
		AddMovementInput(WorldDirection, Scale);
	}
	// ② 攻撃中など：足が止まっている状態
	else
	{
		// ロックオンしている場合は優先してターゲット先に向きを変える
		if (auto Target = LockedOnTarget.Get())
		{
			const FRotator CurrentRot = GetActorRotation();
			const FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			const FRotator TargetRot = FRotator(0.0f, DirectionToTarget.Rotation().Yaw, 0.0f);
			const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), TurnAroundSpeed);
			SetActorRotation(NewRot);
		}
		else
		{
			// 足は止まっているが、初動の向き調整ウィンドウが開いている時だけ手動で回す
			// 即座に回転して良い場合
			if (bCanInstantRotate)
			{
				SetActorRotation(FRotator(0.0f, WorldDirection.Rotation().Yaw, 0.0f));
			}
			else if (bCanSmoothRotate)
			{
				// 滑らかに向きを変える
				const FRotator CurrentRot = GetActorRotation();
				const FRotator TargetRot = FRotator(0.0f, WorldDirection.Rotation().Yaw, 0.0f);
				const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), TurnAroundSpeed);
				SetActorRotation(NewRot);
			}
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

TObjectPtr<AActor> ASamuraiCharacter::FindDeathblowTarget()
{
	TObjectPtr<AActor> Ret;

	// 前方にごく小さいSweepを飛ばして体幹崩れ中タグを持っている敵を探す
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn)};
	const TArray<AActor*> ActorsToIgnore = { this };
	TArray<AActor*> OutActors;
	bool bRet = UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), DeathblowSearchRadius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

	if (!bRet)
		return Ret;

	AActor* BestTarget = nullptr;
	float BestLength = std::numeric_limits<float>::max();
	for (auto Hit : OutActors)
	{
		const FVector ForwardVec =  GetActorForwardVector();
		const FVector DirectionToTargetVec = (Hit->GetActorLocation() - GetActorLocation());
		const float DotProduct = FVector::DotProduct(ForwardVec, DirectionToTargetVec.GetSafeNormal2D());
		
		// 敵が自分の後ろにいる場合はスルー
		if (DotProduct < 0.0f)
			continue;

		const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit);
		if (!ASC)
			continue;

		if (!ASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered))
			continue;

		const float Length = DirectionToTargetVec.Length();
		if (Length > BestLength)
			continue;
		BestLength = Length;
		BestTarget = Hit;
	}

	Ret = BestTarget;

	return Ret;
}

void ASamuraiCharacter::ExecDeathblow(TObjectPtr<AActor> Target)
{
	FGameplayAbilityTargetData_ActorArray* ActorArrayData = new FGameplayAbilityTargetData_ActorArray();
	ActorArrayData->TargetActorArray.Add(Target);

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(ActorArrayData);

	FGameplayEventData Payload;
	Payload.TargetData = TargetDataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CombatGameplayTags::CGTAG_EVENT_Input_Deathblow, Payload);
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
