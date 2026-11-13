// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_DirectHitReact.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_DirectHitReact::UGA_DirectHitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_DirectHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!FrontReactMontage||!BackReactMontage||!LeftReactMontage||!RightReactMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 向きによって前後左右４つのモーションいずれかを再生する
	UAnimMontage* SelectedMontage = [&]()->UAnimMontage*
		{
			const AActor* Attacker = TriggerEventData->Instigator.Get();
			if (!Attacker) return FrontReactMontage;
			const AActor* Receiver = TriggerEventData->Target.Get();
			if (!Receiver) return FrontReactMontage;

			const FVector WorldDirToAttacker = (Attacker->GetActorLocation() - Receiver->GetActorLocation()).GetSafeNormal2D();
			// ワールド空間の向きから自身のローカル空間の向きに変換
			const FVector LocalDir = Receiver->GetActorTransform().InverseTransformVectorNoScale(WorldDirToAttacker);
			const float Angle = LocalDir.Rotation().Yaw;
			UE_LOG(LogTemp, Warning, TEXT("Angle:%f"), Angle);

			if (-45.0f <= Angle && Angle <= 45.0f)
				return FrontReactMontage;
			else if ((-180.0f + 45.0f) < Angle && Angle < -45.0f)
				return LeftReactMontage;
			else if (45.0f < Angle && Angle < (180.0f - 45.0f))
				return RightReactMontage;
			else
				return BackReactMontage;
		}();


	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SelectedMontage
		);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_DirectHitReact::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_DirectHitReact::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_DirectHitReact::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_DirectHitReact::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGA_DirectHitReact::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
