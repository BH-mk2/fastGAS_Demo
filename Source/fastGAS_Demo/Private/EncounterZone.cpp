// Fill out your copyright notice in the Description page of Project Settings.


#include "EncounterZone.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

#include "EnemySpawner.h"
#include "EncounterBarrierInterface.h"

// Sets default values
AEncounterZone::AEncounterZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(400.0f, 400.0f, 200.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void AEncounterZone::OnStageReset_Implementation()
{
	GetWorldTimerManager().ClearTimer(CheckTimerHandle);
	bIsActive = false;

	if (bIsOneTimeOnly && bIsCleared)
	{
		SetBarriersActive(false, true);
		return;
	}

	bIsCleared = false;
	SetBarriersActive(false, true);
}

// Called when the game starts or when spawned
void AEncounterZone::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AEncounterZone::OnTriggerBeginOverlap);

	SetBarriersActive(false, true);
}

void AEncounterZone::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsActive || bIsCleared)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (Character->IsPlayerControlled())
		{
			StartEncounter();
		}
	}
}

// Called every frame
void AEncounterZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEncounterZone::StartEncounter()
{
	bIsActive = true;

	SetBarriersActive(true, false);

	for (AEnemySpawner* Spawner : TargetSpawners)
	{
		if (IsValid(Spawner) && !Spawner->IsEnemyAlive())
		{
			Spawner->SpawnEnemy();
		}
	}

	GetWorldTimerManager().SetTimer(
		CheckTimerHandle,
		this,
		&AEncounterZone::CheckEncounterProgress,
		0.25f,
		true
	);

	OnEncounterCleared.Broadcast();
}

void AEncounterZone::ClearEncounter()
{
	bIsActive = false;
	bIsCleared = true;
	GetWorldTimerManager().ClearTimer(CheckTimerHandle);

	SetBarriersActive(false, false);

	OnEncounterCleared.Broadcast();
}

void AEncounterZone::CheckEncounterProgress()
{
	if (not bIsActive)
	{
		return;
	}

	for (const AEnemySpawner* Spawner : TargetSpawners)
	{
		if (IsValid(Spawner) && Spawner->IsEnemyAlive())
		{
			return;
		}
	}

	ClearEncounter();
}

void AEncounterZone::SetBarriersActive(bool bActive, bool bImmediate)
{
	for (AActor* Barrier : BarrierActors)
	{
		if (!IsValid(Barrier))
		{
			continue;
		}

		if (Barrier->GetClass()->ImplementsInterface(UEncounterBarrierInterface::StaticClass()))
		{
			IEncounterBarrierInterface::Execute_OnSetBarrierActive(Barrier, bActive, bImmediate);
		}
		else
		{
			Barrier->SetActorHiddenInGame(not bActive);
			Barrier->SetActorEnableCollision(bActive);
		}
	}
}

