// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"

#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "CombatGameplayTags.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass || bIsPermanentlyDefeated)
	{
		return;
	}

	if (IsValid(SpawnedEnemy))
	{
		SpawnedEnemy->Destroy();
		SpawnedEnemy = nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(EnemyClass, GetActorTransform(), SpawnParams);
}

bool AEnemySpawner::IsEnemyAlive() const
{
	if (!IsValid(SpawnedEnemy))
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SpawnedEnemy);
	if (ASC && ASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Dead))
	{
		return false;
	}

	return true;
}

void AEnemySpawner::OnStageReset_Implementation()
{
	if (bDoNotRespawnOnceDefeated && !IsEnemyAlive())
	{
		bIsPermanentlyDefeated = true;
		if (IsValid(SpawnedEnemy))
		{
			SpawnedEnemy->Destroy();
			SpawnedEnemy = nullptr;
		}
		return;
	}

	SpawnEnemy();
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (bSpawnOnBeginPlay)
	{
		SpawnEnemy();
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

