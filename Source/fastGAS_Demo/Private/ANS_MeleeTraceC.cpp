// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_MeleeTraceC.h"
#include "CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void UANS_MeleeTraceC::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	UKismetSystemLibrary::PrintString(this, "MeleeTraceC:Start");
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
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp, Start, End, Radius, ObjectTypes, bTraceComplex, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);
	if (bHit)
	{
		for (auto& Hit : OutHits)
		{
			const bool IsAdded = CombatInterface->AddHitActor(Hit.GetActor());
			UKismetSystemLibrary::PrintString(this, FString::Format(TEXT("HitActor:{0}"), { Hit.GetActor()->GetName() }));
			// TODO: 攻撃を当てたときの処理
		}
	}
}
