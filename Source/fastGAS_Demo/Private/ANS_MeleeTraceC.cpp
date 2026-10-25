// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_MeleeTraceC.h"
#include "CombatInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CombatGameplayTags.h"

#include "Kismet/KismetSystemLibrary.h"

void UANS_MeleeTraceC::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	// 各種データの初期化
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(MeshComp->GetOwner());
	if (!CombatInterface)
	{
		return;
	}

	CombatInterface->ResetHitActors();
}
void UANS_MeleeTraceC::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	auto* Owner = MeshComp->GetOwner();
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Owner);
	if (!CombatInterface)
	{
		return;
	}

	FVector Start, End;
	const bool Result = CombatInterface->GetAttackPoints(Start, End);
	if (!Result)
	{
		return;
	}

	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	constexpr bool bTraceComplex = false;
	const TArray<AActor*> ActorsToIgnore = {Owner};
	TArray<FHitResult> OutHits;

	// 武器のソケットを元にしたSweepで衝突判定をする
	// Memo: WorldContextObjectにthis(UAnimNotifyState)を指定すると衝突判定が行われない(ように見える)
	//       存在しているWorldが確実なMeshCompを指定することで解消
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp, Start, End, Radius, ObjectTypes, bTraceComplex, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);
	if (bHit)
	{
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
		if (!SourceASC)
		{
			ensureMsgf(SourceASC!=nullptr, TEXT("[ANS_MeleeTrace] Owner has not ASC."));
			return;
		}

		for (auto& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			const bool IsAdded = CombatInterface->AddHitActor(HitActor);

			if (!IsAdded)
				continue;

			// 直撃時の処理
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			if (!TargetASC)
				continue;

			FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
			EffectContext.AddHitResult(Hit);
			EffectContext.AddInstigator(Owner, Owner);

			if (!DamageEffect)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ANS_MeleeTrace] DamageEffect is not set on %s"), *GetName());
				return;
			}

			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, EffectContext);
			if (!SpecHandle.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("[ANS_MeleeTrace] Failed to MakeOutgoingSpec"));
				return;
			}
			SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Health, DamageHealthMagnitude);
			SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture, DamagePostureMagnitude);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}
