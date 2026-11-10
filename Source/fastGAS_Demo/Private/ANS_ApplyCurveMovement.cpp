// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_ApplyCurveMovement.h"
#include "GameFramework/Character.h"

void UANS_ApplyCurveMovement::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	if (!AnimInst)
		return;

	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	const float CurrentCurveValue = AnimInst->GetCurveValue(CurveName);
	if (FMath::IsNearlyZero(CurrentCurveValue))
		return;

	const float Speed = CurrentCurveValue * SpeedMultiplier;
	const FRotator TargetRot = Character->GetActorRotation() + FRotator(0.0f, DirectionAngle, 0.0f);
	const FVector MoveDelta = TargetRot.Vector() * Speed * FrameDeltaTime;

	FHitResult HitResult;
	Character->AddActorWorldOffset(MoveDelta, true, &HitResult);
}
