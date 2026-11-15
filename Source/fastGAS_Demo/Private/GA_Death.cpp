// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Death.h"

#include "CombatGameplayTags.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <Components/CapsuleComponent.h>
#include <AIController.h>
#include "BrainComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_Death::UGA_Death()
{
	FAbilityTriggerData AbilityTriggerData;
	AbilityTriggerData.TriggerTag = CombatGameplayTags::CGTAG_EVENT_Combat_HealthBroken;
	AbilityTriggerData.TriggerSource = {EGameplayAbilityTriggerSource::GameplayEvent};
	AbilityTriggers.Add(AbilityTriggerData);

	ActivationOwnedTags.AddTag(CombatGameplayTags::CGTAG_STATE_Dead);
	ActivationBlockedTags.AddTag(CombatGameplayTags::CGTAG_STATE_Dead);
	CancelAbilitiesWithTag.AddTag(CombatGameplayTags::CGTAG_ABILITY);
}

void UGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 移動・衝突の停止
	if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->DisableMovement();
	}
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// AI・コントローラの入力無効化
	if (AAIController* AIC = Cast<AAIController>(Character->GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}
	else if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		Character->DisableInput(PC);
	}

	// 死亡モンタージュの再生
	UAnimMontage* MontageToPlay = DefaultDeathMontage;
	if (TriggerEventData && TriggerEventData->InstigatorTags.HasTag(CombatGameplayTags::CGTAG_DAMAGE_Type_Deathblow))
	{
		if (DeathblowDeathMontage)
			MontageToPlay = DeathblowDeathMontage;
	}

	if (!MontageToPlay)
	{
		OnDeathMontageFinished();
	}
	else
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay, 1.0f, NAME_None, false
		);

		Task->OnCompleted.AddDynamic(this, &UGA_Death::OnDeathMontageFinished);
		Task->OnInterrupted.AddDynamic(this, &UGA_Death::OnDeathMontageFinished);
		Task->OnBlendOut.AddDynamic(this, &UGA_Death::OnDeathMontageFinished);
		Task->OnCancelled.AddDynamic(this, &UGA_Death::OnDeathMontageFinished);
		Task->ReadyForActivation();
	}
}

void UGA_Death::OnDeathMontageFinished()
{
	// EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
