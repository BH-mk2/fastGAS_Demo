// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiAIController.h"

#include "SamuraiCharacter.h"

ASamuraiAIController::ASamuraiAIController()
{
	bStartAILogicOnPossess = true;
}

void ASamuraiAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ASamuraiCharacter* OwnerSamurai = Cast<ASamuraiCharacter>(GetOwner()))
	{
		OwnerSamurai->SetLockOnTarget(nullptr);
	}
}

void ASamuraiAIController::SetFocus(AActor* NewFocus, EAIFocusPriority::Type InPriority)
{
	Super::SetFocus(NewFocus, InPriority);

	if (AActor* TargetSamurai = GetFocusActor())
	{
		if (ASamuraiCharacter* OwnerSamurai = Cast<ASamuraiCharacter>(GetOwner()))
		{
			OwnerSamurai->SetLockOnTarget(TargetSamurai);
		}
	}

}

void ASamuraiAIController::ClearFocus(EAIFocusPriority::Type InPriority)
{
	Super::ClearFocus(InPriority);

	if (ASamuraiCharacter* OwnerSamurai = Cast<ASamuraiCharacter>(GetOwner()))
	{
		OwnerSamurai->SetLockOnTarget(nullptr);
	}
}

void ASamuraiAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	// 何もしないことで独自の回転処理を邪魔しないようにする
}
