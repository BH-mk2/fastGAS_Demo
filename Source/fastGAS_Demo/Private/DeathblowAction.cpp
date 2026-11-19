// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathblowAction.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "MovieSceneSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "DefaultLevelSequenceInstanceData.h"

#include "CombatGameplayTags.h"

void UDeathblowAction::Execute_Implementation(ACharacter* Attacker, ACharacter* Victim)
{
	CachedAttacker = Attacker;
	CachedVictim = Victim;
}

UWorld* UDeathblowAction::GetWorld() const
{
	if (CachedAttacker.IsValid())
	{
		return CachedAttacker->GetWorld();
	}
	if (const UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
	return nullptr;
}

void UDeathblowAction::FinishAction()
{
	OnFinished.Broadcast();
}

void UDeathblowAction::ApplyResultEffect()
{
	if (bEffectApplied)
		return;

	if (!CachedAttacker.IsValid() || !CachedVictim.IsValid())
		return;

	if (DeathblowDamageEffect)
	{
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedAttacker.Get());
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedVictim.Get());
		if (SourceASC && TargetASC)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DeathblowDamageEffect, 1.0f, Context);
			if (Spec.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
				bEffectApplied = true;
			}
		}
	}
}

void UDeathblowAction_Standard::Execute_Implementation(ACharacter* Attacker, ACharacter* Victim)
{
	Super::Execute_Implementation(Attacker, Victim);

	if (!Attacker || !Victim)
	{
		FinishAction();
		return;
	}

	bEffectApplied = false;

	// 1. 位置・向きの同期（現在 GA_Deathblow に書いている位置合わせ処理があればここへ移植）
	const FVector TargetLocation = Victim->GetActorLocation() + Victim->GetActorForwardVector() * FrontDistance;
	Attacker->SetActorLocation(TargetLocation);
	const FRotator LookAtVictim = (Victim->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D().Rotation();
	Attacker->SetActorRotation(LookAtVictim);

	// 2. 被害者側の演出モンタージュ再生（ある場合）
	if (VictimMontage)
	{
		Victim->PlayAnimMontage(VictimMontage);
	}

	// 3. インパクト通知待ち
	if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker))
	{
		ImpactEventHandle = SourceASC->GenericGameplayEventCallbacks
			.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_Deathblow_Impact)
			.AddUObject(this, &UDeathblowAction_Standard::OnImpactEventReceived);
	}

	// 4. カメラズーム開始
	if (bEnableCameraZoom)
	{
		StartCameraZoom(Attacker);
	}

	// 4. 攻撃者側のモンタージュ再生と終了バインド
	UAnimInstance* AnimInstance = Attacker->GetMesh() ? Attacker->GetMesh()->GetAnimInstance() : nullptr;
	if (AttackerMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(AttackerMontage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UDeathblowAction_Standard::OnAttackerMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackerMontage);
	}
	else
	{
		FinishAction();
	}
}

void UDeathblowAction_Standard::FinishAction()
{
	if (!bEffectApplied)
	{
		OnImpactEventReceived(nullptr);
		bEffectApplied = true;
	}

	RestoreCamera();

	if (CachedAttacker.IsValid() && ImpactEventHandle.IsValid())
	{
		if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedAttacker.Get()))
		{
			SourceASC->GenericGameplayEventCallbacks
				.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_Deathblow_Impact)
				.Remove(ImpactEventHandle);
		}
		ImpactEventHandle.Reset();
	}
	Super::FinishAction();
}

void UDeathblowAction_Standard::OnAttackerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	FinishAction();
}

void UDeathblowAction_Standard::OnImpactEventReceived(const FGameplayEventData* Payload)
{
	// カメラを揺らす
	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
	{
		CameraManager->StartCameraShake(
			ShakeClass,
			1.0f,
			ECameraShakePlaySpace::CameraLocal,
			FRotator::ZeroRotator
		);
	}

	// ダメージGEの適用
	ApplyResultEffect();
}

void UDeathblowAction_Standard::StartCameraZoom(ACharacter* Attacker)
{
	USpringArmComponent* SpringArm = Attacker->FindComponentByClass<USpringArmComponent>();
	UCameraComponent* Camera = Attacker->FindComponentByClass<UCameraComponent>();
	if (!SpringArm || !Camera)
	{
		return;
	}

	if (!GetWorld()->GetTimerManager().IsTimerActive(CameraTimerHandle))
	{
		OriginalArmLength = SpringArm->TargetArmLength;
		OriginalSocketOffset = SpringArm->SocketOffset;
		OriginalFOV = Camera->FieldOfView;
	}

	CachedSpringArm = SpringArm;
	CachedCamera = Camera;
	bIsZoomingIn = true;

	GetWorld()->GetTimerManager().SetTimer(
		CameraTimerHandle,
		this,
		&UDeathblowAction_Standard::UpdateCameraInterp,
		0.016f,
		true
	);
}

void UDeathblowAction_Standard::UpdateCameraInterp()
{
	UWorld* World = GetWorld();
	if (!World || !CachedSpringArm.IsValid() || !CachedCamera.IsValid())
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(CameraTimerHandle);
		}
		return;
	}

	// フラグがONの場合は目標値に近づいて、OFFの場合はオリジナルの値に近づく
	const float DeltaTime = World->GetDeltaSeconds();
	const float TargetArm = bIsZoomingIn ? ZoomArmLength : OriginalArmLength;
	const FVector TargetOffset = bIsZoomingIn ? ZoomSocketOffset : OriginalSocketOffset;
	const float TargetFOV = bIsZoomingIn ? ZoomFOV : OriginalFOV;

	CachedSpringArm->TargetArmLength = FMath::FInterpTo(CachedSpringArm->TargetArmLength, TargetArm, DeltaTime, CameraInterpSpeed);
	CachedSpringArm->SocketOffset = FMath::VInterpTo(CachedSpringArm->SocketOffset, TargetOffset, DeltaTime, CameraInterpSpeed);
	CachedCamera->SetFieldOfView(FMath::FInterpTo(CachedCamera->FieldOfView, TargetFOV, DeltaTime, CameraInterpSpeed));

	// ズーム演出が終了して元々のカメラ設定値に近づいたらズーム演出を終了
	if (!bIsZoomingIn)
	{
		const bool bArmDone = FMath::IsNearlyEqual(CachedSpringArm->TargetArmLength, OriginalArmLength, 0.5f);
		const bool bFOVDone = FMath::IsNearlyEqual(CachedCamera->FieldOfView, OriginalFOV, 0.2f);
		if (bArmDone && bFOVDone)
		{
			CachedSpringArm->TargetArmLength = OriginalArmLength;
			CachedSpringArm->SocketOffset = OriginalSocketOffset;
			CachedCamera->SetFieldOfView(OriginalFOV);
			World->GetTimerManager().ClearTimer(CameraTimerHandle);
		}
	}
}

void UDeathblowAction_Standard::RestoreCamera()
{
	bIsZoomingIn = false;
}

void UDeathblowAction_Cinematic::Execute_Implementation(ACharacter* Attacker, ACharacter* Victim)
{
	Super::Execute_Implementation(Attacker, Victim);

	UWorld* World = GetWorld();
	if (!World || !Attacker || !Victim || !DeathblowSequence)
	{
		FinishAction();
		return;
	}

	World->GetTimerManager().ClearTimer(CleanupTimerHandle);
	CleanupSequenceActor();

	bEffectApplied = false;

	// インパクト用イベント待ち
	if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker))
	{
		ImpactEventHandle = SourceASC->GenericGameplayEventCallbacks
			.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_Deathblow_Impact)
			.AddUObject(this, &UDeathblowAction_Cinematic::OnImpactEventReceived);
	}

	// LevelSequencePlayerの作成
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bAutoPlay = false;

	ALevelSequenceActor* OutActor = nullptr;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(World, DeathblowSequence, Settings, OutActor);
	if (!SequencePlayer || !OutActor)
	{
		OnSequenceFinished();
		return;
	}

	CachedSequenceActor = OutActor;

	// シーケンス内のタグに Attacker / Victimをバインド
	if (!AttackerBindingTag.IsNone())
	{
		OutActor->SetBindingByTag(AttackerBindingTag, { Attacker });
	}
	if (!VictimBindingTag.IsNone())
	{
		OutActor->SetBindingByTag(VictimBindingTag, { Victim });
	}

	// 再生基準位置をターゲットの位置・向きに合わせる
	if (bUseVictimTransformOrigin)
	{
		FTransform OriginTransform = Victim->GetActorTransform();
		if (UCapsuleComponent* Capsule = Victim->GetCapsuleComponent())
		{
			FVector FootLocation = OriginTransform.GetLocation();
			FootLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
			OriginTransform.SetLocation(FootLocation);
		}

		OutActor->bOverrideInstanceData = true;
		if(UDefaultLevelSequenceInstanceData* InstanceData = Cast<UDefaultLevelSequenceInstanceData>(OutActor->DefaultInstanceData))
		{
			InstanceData->TransformOrigin = OriginTransform;
		}
	}

	Attacker->SetActorEnableCollision(false);
	Victim->SetActorEnableCollision(false);

	if (UCharacterMovementComponent* AttackerMove = Attacker->GetCharacterMovement())
	{
		AttackerMove->StopMovementImmediately();
		AttackerMove->SetMovementMode(MOVE_None);
	}

	if (UCharacterMovementComponent* VictimMove = Victim->GetCharacterMovement())
	{
		VictimMove->StopMovementImmediately();
		VictimMove->SetMovementMode(MOVE_None);
	}

	// シーケンス完了イベントのバインドと再生開始
	SequencePlayer->OnFinished.AddDynamic(this, &UDeathblowAction_Cinematic::OnSequenceFinished);
	SequencePlayer->Play();
}

void UDeathblowAction_Cinematic::FinishAction()
{
	if (!bEffectApplied)
	{
		OnImpactEventReceived(nullptr);
		bEffectApplied = true;
	}
	Super::FinishAction();
}

void UDeathblowAction_Cinematic::OnImpactEventReceived(const FGameplayEventData* Payload)
{
	ApplyResultEffect();
}

void UDeathblowAction_Cinematic::OnSequenceFinished()
{
	if (CachedAttacker.IsValid() && ImpactEventHandle.IsValid())
	{
		if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedAttacker.Get()))
		{
			SourceASC->GenericGameplayEventCallbacks
				.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_Deathblow_Impact)
				.Remove(ImpactEventHandle);
		}
		ImpactEventHandle.Reset();
	}

	if (CachedAttacker.IsValid())
	{
		CachedAttacker->SetActorEnableCollision(true);
		if (UCharacterMovementComponent* AttackerMove = CachedAttacker->GetCharacterMovement())
		{
			AttackerMove->SetMovementMode(MOVE_Walking);
		}
	}

	if (CachedVictim.IsValid())
	{
		// 敵がまだ生きている場合（ボス1ゲージ目削りなど）のみコリジョンと移動を復帰
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedVictim.Get());
		const bool bIsDead = TargetASC && TargetASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Dead);
		if (!bIsDead)
		{
			CachedVictim->SetActorEnableCollision(true);
			if (UCharacterMovementComponent* VictimMove = CachedVictim->GetCharacterMovement())
			{
				VictimMove->SetMovementMode(MOVE_Walking);
			}
		}
	}

	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.RemoveDynamic(this, &UDeathblowAction_Cinematic::OnSequenceFinished);
		SequencePlayer = nullptr;
	}

	if (CachedAttacker.IsValid())
	{
		if (APlayerController* PC = Cast<APlayerController>(CachedAttacker->GetController()))
		{
			if (bSyncControlRotation)
			{
				if (AActor* CurrentViewTarget = PC->GetViewTarget())
				{
					FRotator EndCamRot = CurrentViewTarget->GetActorRotation();
					EndCamRot.Roll = 0.0f;
					PC->SetControlRotation(EndCamRot);
				}
			}
			PC->SetViewTargetWithBlend(CachedAttacker.Get(), CameraBlendOutSeconds, VTBlend_Cubic);
		}
	}

	if (UWorld* World = GetWorld())
	{
		const float DestroyDelay = FMath::Max(CameraBlendOutSeconds + 0.05f, 0.05f);
		World->GetTimerManager().SetTimer(
			CleanupTimerHandle,
			this,
			&UDeathblowAction_Cinematic::CleanupSequenceActor,
			DestroyDelay,
			false
		);
	}

	FinishAction();
}

void UDeathblowAction_Cinematic::CleanupSequenceActor()
{
	if (CachedSequenceActor.IsValid())
	{
		CachedSequenceActor->Destroy();
		CachedSequenceActor.Reset();
	}
}
