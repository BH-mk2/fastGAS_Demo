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
		HitProcess(Owner, CombatInterface, OutHits);
	}
}

void UANS_MeleeTraceC::HitProcess(AActor* Owner, ICombatInterface* CombatInterface, TArray<FHitResult>& Hits)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!SourceASC)
	{
		ensureMsgf(SourceASC != nullptr, TEXT("[ANS_MeleeTrace] Owner has not ASC."));
		return;
	}

	for (auto& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		const bool IsAdded = CombatInterface->AddHitActor(HitActor);

		if (!IsAdded)
			continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC)
			continue;

		// 自分と相手の向きによってガード/弾き/直撃成立の処理を分岐する
		EHitKind HitKind = CheckHitResult(*Owner, *SourceASC, *HitActor, *TargetASC);
		FInnerProcessor InnerProcessor(*this, *Owner, *SourceASC, *HitActor, *TargetASC, Hit);
		switch (HitKind)
		{
		case UANS_MeleeTraceC::EHitKind::Direct:
			InnerProcessor.ProcessDirect();
			break;
		case UANS_MeleeTraceC::EHitKind::Guard:
			InnerProcessor.ProcessGuard();
			break;
		case UANS_MeleeTraceC::EHitKind::Parry:
			InnerProcessor.ProcessParry();
			break;
		default:
			break;
		}
	}
}

UANS_MeleeTraceC::EHitKind UANS_MeleeTraceC::CheckHitResult(AActor& Src, UAbilitySystemComponent& SrcASC, AActor& Target, UAbilitySystemComponent& TargetASC)
{
	// 互いの向きが正面/背後か判定
	const bool bIsFront = [&]()
		{
			const FVector ToSource = (Src.GetActorLocation() - Target.GetActorLocation()).GetSafeNormal2D();
			const FVector TargetForward = Target.GetActorForwardVector().GetSafeNormal2D();
			const float DotProduct = FVector::DotProduct(ToSource, TargetForward);

			return DotProduct > 0.0f;
		}();

	// 背後の場合は直撃成立
	if (!bIsFront)
		return EHitKind::Direct;

	// 正面かつ弾きタグを持っている場合は弾き成立
	const bool bIsParrying = TargetASC.HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Parry);

	if (bIsParrying)
		return EHitKind::Parry;

	// 正面かつガード判定タグを持っている場合はガード成立
	const bool bIsGuarding = TargetASC.HasMatchingGameplayTag(CombatGameplayTags::CGTAG_COMBAT_Defense_Guard);

	if (bIsGuarding)
		return EHitKind::Guard;

	return EHitKind::Direct;
}

UANS_MeleeTraceC::FInnerProcessor::FInnerProcessor(UANS_MeleeTraceC& Context, AActor& Src, UAbilitySystemComponent& SrcASC, AActor& Target, UAbilitySystemComponent& TargetASC, FHitResult& HitResult)
	:Context(Context), Src(Src), SrcASC(SrcASC), Target(Target), TargetASC(TargetASC), HitResult(HitResult)
{
}

void UANS_MeleeTraceC::FInnerProcessor::ProcessDirect()
{
	// 相手にダメージを与える
	FGameplayEffectContextHandle EffectContext = SrcASC.MakeEffectContext();
	EffectContext.AddInstigator(&Src, &Src);
	EffectContext.AddHitResult(HitResult);

	FGameplayEffectSpecHandle SpecHandle = SrcASC.MakeOutgoingSpec(Context.DirectHitEffect, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Health, Context.DamageHealthMagnitude);
		SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture, Context.DamagePostureMagnitude);
		SrcASC.ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), &TargetASC);
	}

	{
		FGameplayEventData EventData;
		EventData.EventTag = CombatGameplayTags::CGTAG_EVENT_Combat_DirectHitReact;
		EventData.Instigator = &Src;
		EventData.Target = &Target;
		EventData.ContextHandle = EffectContext;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			&Target,
			CombatGameplayTags::CGTAG_EVENT_Combat_DirectHitReact,
			EventData
		);
	}
}

void UANS_MeleeTraceC::FInnerProcessor::ProcessGuard()
{
	// 自分はガードされて、相手はガードできた状態

	const float PostureValue = Context.DamagePostureMagnitude / 2.0f;

	// 攻撃を防いだイベントを発行
	{
		FGameplayEventData Payload;
		Payload.Instigator = &Src;
		Payload.Target = &Target;
		Payload.EventMagnitude = PostureValue;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(&Target, CombatGameplayTags::CGTAG_EVENT_Combat_HitBlock, Payload);
	}

	// 相手に体幹ダメージを与える
	{
		FGameplayEffectContextHandle EffectContext = SrcASC.MakeEffectContext();
		EffectContext.AddInstigator(&Src, &Src);
		EffectContext.AddHitResult(HitResult);

		FGameplayEffectSpecHandle SpecHandle = SrcASC.MakeOutgoingSpec(Context.HitBlockedEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// TODO: 半減率は後でキャラのパラメータに変える
			SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture, PostureValue);
			SrcASC.ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), &TargetASC);
		}
	}
}

void UANS_MeleeTraceC::FInnerProcessor::ProcessParry()
{
	// 自分は弾かれて、相手は弾いた状態

	// 攻撃を防いだイベントを発行
	{
		FGameplayEventData Payload;
		Payload.Instigator = &Src;
		Payload.Target = &Target;
		Payload.EventMagnitude = 0;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(&Target, CombatGameplayTags::CGTAG_EVENT_Combat_HitBlock, Payload);
	}

	// 自分に体幹ダメージを与える
	{
		FGameplayEffectContextHandle EffectContext = SrcASC.MakeEffectContext();
		EffectContext.AddInstigator(&Src, &Src);
		EffectContext.AddHitResult(HitResult);

		FGameplayEffectSpecHandle SpecHandle = SrcASC.MakeOutgoingSpec(Context.ParryEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// TODO: 半減率は後でキャラのパラメータに変える
			const float PostureValue = Context.DamagePostureMagnitude / 2.0f;
			SpecHandle.Data->SetSetByCallerMagnitude(CombatGameplayTags::CGTAG_DATA_Damage_Posture, PostureValue);
			SrcASC.ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}
