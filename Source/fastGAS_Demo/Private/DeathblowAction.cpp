// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathblowAction.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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


void UDeathblowAction_Standard::Execute_Implementation(ACharacter* Attacker, ACharacter* Victim)
{
	Super::Execute_Implementation(Attacker, Victim);

	if (!Attacker || !Victim)
	{
		FinishAction();
		return;
	}

	bDamageApplied = false;

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
	if (!bDamageApplied)
	{
		OnImpactEventReceived(nullptr);
		bDamageApplied = true;
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
				bDamageApplied = true;
			}
		}
	}
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
