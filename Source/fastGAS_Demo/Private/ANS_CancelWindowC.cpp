// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_CancelWindowC.h"
#include "AbilitySystemComponent.h"
#include "CombatGameplayTags.h"

void UANS_CancelWindowC::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	auto* Owner = MeshComp->GetOwner();
	// 指定したCancellableタグを付与する
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UANS_CancelWindowC] ASC is nothing."));
		return;
	}

	ASC->AddLooseGameplayTags(CancelTags);

	// 先行入力の評価イベントを送信
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, CombatGameplayTags::CGTAG_EVENT_Input_Evaluate, FGameplayEventData{});
}

void UANS_CancelWindowC::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	auto* Owner = MeshComp->GetOwner();
	// 指定したCancellableタグを外す
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UANS_CancelWindowC] ASC is nothing."));
		return;
	}

	ASC->RemoveLooseGameplayTags(CancelTags);
}
