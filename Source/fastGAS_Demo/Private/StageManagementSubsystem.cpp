// Fill out your copyright notice in the Description page of Project Settings.


#include "StageManagementSubsystem.h"

#include "EngineUtils.h"

#include "StageResettable.h"

void UStageManagementSubsystem::SetRespawnTransform(const FTransform& NewTransform)
{
	CurrentRespawnTransform = NewTransform;
}

bool UStageManagementSubsystem::GetRespawnTransform(FTransform& OutTransform) const
{
	if (!CurrentRespawnTransform)
		return false;

	OutTransform = CurrentRespawnTransform.value();
	return true;
}

void UStageManagementSubsystem::ResetStage()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// ワールド内のIStageResettableを実装した全アクターにリセットを通知
	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) && Actor->GetClass()->ImplementsInterface(UStageResettable::StaticClass()))
		{
			IStageResettable::Execute_OnStageReset(Actor);
		}
	}

	OnStageResetDelegate.Broadcast();
}
