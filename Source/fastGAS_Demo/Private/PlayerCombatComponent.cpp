// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "SamuraiCharacter.h"
#include "CombatGameplayTags.h"

// Sets default values for this component's properties
UPlayerCombatComponent::UPlayerCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerSamurai = Cast<ASamuraiCharacter>(GetOwner());
	if (!OwnerSamurai)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = OwnerSamurai->GetAbilitySystemComponent())
	{
		ASC->GenericGameplayEventCallbacks
			.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_HealthBroken)
			.AddUObject(this, &UPlayerCombatComponent::OnPlayerHealthBroken);
	}
}

void UPlayerCombatComponent::ToggleLockOnTarget()
{
	if (!OwnerSamurai)
	{
		return;
	}

	// 既にロックオンしている場合、外して終了
	TWeakObjectPtr<AActor> CurrentLockedOnTarget = OwnerSamurai->GetLockedOnTarget();
	if (CurrentLockedOnTarget.IsValid())
	{
		OwnerSamurai->LockOff();
		return;
	}

	// ターゲットを探索して、存在したらロックオンする
	TWeakObjectPtr<AActor> Target = SearchTarget();
	if (!Target.IsValid())
	{
		// 存在しないので終了
		OwnerSamurai->SetLockOnTarget(nullptr);
		return;
	}

	OwnerSamurai->SetLockOnTarget(Target);
}

TWeakObjectPtr<AActor> UPlayerCombatComponent::SearchTarget()
{
	if (!OwnerSamurai)
	{
		return nullptr;
	}

	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	constexpr bool bTraceComplex = false;
	const TArray<AActor*> ActorsToIgnore = { OwnerSamurai };
	TArray<FHitResult> OutHits;

	// Sweepをカメラの前方に飛ばして一番近いターゲットにロックオンする
	// 1. 球を飛ばす
	const FVector Start = OwnerSamurai->GetActorLocation();
	const FVector End = Start;
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, LockOnRadius, ObjectTypes, bTraceComplex, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);

	// 2. カメラ前方にいるものだけにフィルターする
	const FVector ViewForward = OwnerSamurai->GetViewRotation().Vector();

	double BestLength = -1.0f;
	AActor* BestTarget = nullptr;

	for (FHitResult& Enemy : OutHits)
	{
		const auto TargetActor = Enemy.GetActor();
		if (!TargetActor)continue;
		const FVector ForwardVec = ViewForward;
		const FVector ToTargetVec = TargetActor->GetActorLocation() - OwnerSamurai->GetActorLocation();
		const double ToTargetLength = ToTargetVec.Length();
		const FVector ToTargetNormal = ToTargetVec.GetSafeNormal2D();
		const float DotProduct = FVector::DotProduct(ForwardVec, ToTargetNormal);

		// 正面ではないものは無視
		if (DotProduct < 0.0f)
			continue;

		if (ToTargetLength > BestLength)
		{
			BestLength = ToTargetLength;
			BestTarget = TargetActor;
		}
	}

	return BestTarget;
}

void UPlayerCombatComponent::OnPlayerHealthBroken(const FGameplayEventData* Payload)
{
	if (!OwnerSamurai)
	{
		return;
	}

	UAbilitySystemComponent* ASC = OwnerSamurai->GetAbilitySystemComponent();
	if (!ASC || ASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Dead))
	{
		return;
	}

	FGameplayEventData DeathPayload = Payload ? *Payload : FGameplayEventData();
	DeathPayload.EventTag = CombatGameplayTags::CGTAG_EVENT_Combat_ExecuteDeath;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerSamurai,
		CombatGameplayTags::CGTAG_EVENT_Combat_ExecuteDeath,
		DeathPayload
	);
}
