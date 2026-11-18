// Fill out your copyright notice in the Description page of Project Settings.


#include "SlidingEncounterBarrier.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ASlidingEncounterBarrier::ASlidingEncounterBarrier()
{
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(BlockerBox);
	GateMesh->SetMobility(EComponentMobility::Movable);
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASlidingEncounterBarrier::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	if (GateMesh)
	{
		GateMesh->SetRelativeLocation(bPreviewClosedInEditor ? ClosedLocation : OpenLocation);
	}
#endif
}

void ASlidingEncounterBarrier::UpdateBarrierProgress_Implementation(float Alpha)
{
	const FVector NewLocation = FMath::Lerp(OpenLocation, ClosedLocation, Alpha);
	GateMesh->SetRelativeLocation(NewLocation);
}
