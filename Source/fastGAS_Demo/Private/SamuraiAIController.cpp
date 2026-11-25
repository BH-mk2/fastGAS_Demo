// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiAIController.h"

#include "SamuraiCharacter.h"

ASamuraiAIController::ASamuraiAIController()
{
	bStartAILogicOnPossess = true;
	SetGenericTeamId(FGenericTeamId(1));
}

void ASamuraiAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ASamuraiCharacter* OwnerSamurai = Cast<ASamuraiCharacter>(InPawn))
	{
		OwnerSamurai->SetLockOnTarget(nullptr);
	}
}

void ASamuraiAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ASamuraiAIController::SetCombatFocus(AActor* Target)
{
	if (ASamuraiCharacter* OwnerSamurai = Cast<ASamuraiCharacter>(GetPawn()))
	{
		OwnerSamurai->SetLockOnTarget(Target);
	}
}

void ASamuraiAIController::ClearCombatFocus(AActor* ForgottenActor)
{
	if (ASamuraiCharacter* OwnerSamurai = Cast<ASamuraiCharacter>(GetPawn()))
	{
		if (ForgottenActor && ForgottenActor != OwnerSamurai->GetLockedOnTarget())
		{
			return;
		}
		OwnerSamurai->SetLockOnTarget(nullptr);
	}
}

void ASamuraiAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	// アクターの独自の回転処理を邪魔しないようにする

	// 視覚の向きは制御する
	if (APawn* MyPawn = GetPawn())
	{
		SetControlRotation(MyPawn->GetActorRotation());
	}
}
