// Fill out your copyright notice in the Description page of Project Settings.


#include "EncounterBarrier.h"
#include "Components/BoxComponent.h"

// Sets default values
AEncounterBarrier::AEncounterBarrier()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BlockerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockerBox"));
	SetRootComponent(BlockerBox);
	BlockerBox->SetBoxExtent(FVector(40.0f, 200.0f, 400.0f));
	BlockerBox->SetCollisionProfileName(TEXT("BlockAll"));
}

void AEncounterBarrier::OnSetBarrierActive_Implementation(bool bActive, bool bImmediate)
{
	BlockerBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

	if (bImmediate)
	{
		StopBarrierTransition(bActive);
		UpdateBarrierProgress(bActive ? 1.0f : 0.0f);
		return;
	}

	PlayBarrierTransition(bActive);
}

// Called when the game starts or when spawned
void AEncounterBarrier::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEncounterBarrier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

