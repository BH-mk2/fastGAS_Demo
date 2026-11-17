// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"

#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#if WITH_EDITORONLY_DATA
#include "Components/CapsuleComponent.h"
#endif

#include "CombatGameplayTags.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

#if WITH_EDITORONLY_DATA
	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(Root);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetHiddenInGame(true);
	PreviewMesh->bIsEditorOnly = true;
#endif
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

void AEnemySpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	if (!PreviewMesh)
	{
		return;
	}

	if (EnemyClass)
	{
		if (const ACharacter* DefaultChar = EnemyClass->GetDefaultObject<ACharacter>())
		{
			if (const USkeletalMeshComponent* DefaultMesh = DefaultChar->GetMesh())
			{
				PreviewMesh->SetSkeletalMesh(DefaultMesh->GetSkeletalMeshAsset());

				FTransform MeshTransform = DefaultMesh->GetRelativeTransform();
				if (const UCapsuleComponent* Capsule = DefaultChar->GetCapsuleComponent())
				{
					MeshTransform.AddToTranslation(FVector(0.0f, 0.0f, Capsule->GetScaledCapsuleHalfHeight()));
				}
				PreviewMesh->SetRelativeTransform(MeshTransform);

				for (int32 i = 0; i < DefaultMesh->GetNumMaterials(); ++i)
				{
					PreviewMesh->SetMaterial(i, DefaultMesh->GetMaterial(i));
				}
				return;
			}
		}
	}

	PreviewMesh->SetSkeletalMesh(nullptr);
#endif
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

