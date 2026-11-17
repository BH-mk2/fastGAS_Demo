// Fill out your copyright notice in the Description page of Project Settings.


#include "CheckPointActor.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "StageManagementSubsystem.h"

// Sets default values
ACheckPointActor::ACheckPointActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(Root);
	InteractionSphere->SetSphereRadius(150.0f);

	RespawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(Root);
}

void ACheckPointActor::ActivateCheckpoint(APawn* PlayerPawn)
{
	if (not IsValid(PlayerPawn))
	{
		return;
	}

	// サブシステムに復活地点を登録
	if (UStageManagementSubsystem* StageSubsystem = GetWorld()->GetSubsystem<UStageManagementSubsystem>())
	{
		StageSubsystem->SetRespawnTransform(RespawnPoint->GetComponentTransform());

		// プレイヤーのパラメータを全快
		if (RestorePlayerGE)
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn))
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(this);
				ASC->ApplyGameplayEffectToSelf(RestorePlayerGE.GetDefaultObject(), 1.0f, Context);
			}
		}

		StageSubsystem->ResetStage();
	}
}

// Called when the game starts or when spawned
void ACheckPointActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACheckPointActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

