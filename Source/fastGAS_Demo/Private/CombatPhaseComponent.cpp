// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPhaseComponent.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

#include "CombatGameplayTags.h"
#include "CombatAttributeSet.h"

const FEnemyPhaseConfig UCombatPhaseComponent::DefaultPhaseConfig = FEnemyPhaseConfig();

// Sets default values for this component's properties
UCombatPhaseComponent::UCombatPhaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UCombatPhaseComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializePhaseSystem();
}

void UCombatPhaseComponent::ConsumeStockByDeathblow(const FGameplayEventData& DeathblowPayload)
{
	if (!OwnerASC || !OwnerAttributeSet)
		return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaggerRecoveryTimerHandle);
	}
	CurrentStaggerReason = ECombatStaggerReason::None;
	OwnerASC->RemoveLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered);

	const int32 CurrentStocks = FMath::RoundToInt(OwnerAttributeSet->GetRemainingStocks());
	if (CurrentStocks > 1)
	{
		// ストックを１消費して次のゲージ用にパラメータをセット
		OwnerAttributeSet->SetRemainingStocks(static_cast<float>(CurrentStocks - 1));
		OwnerAttributeSet->SetHealth(OwnerAttributeSet->GetMaxHealth());
		OwnerAttributeSet->SetPosture(0.0f);

		ApplyPhaseConfig(CurrentPhaseIndex + 1, false);
	}
	else
	{
		// 完全撃破の場合
		OwnerAttributeSet->SetRemainingStocks(0.0f);
		OwnerAttributeSet->SetHealth(0.0f);

		FGameplayEventData DeathPayload = DeathblowPayload;
		DeathPayload.EventTag = CombatGameplayTags::CGTAG_EVENT_Combat_ExecuteDeath;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwner(),
			CombatGameplayTags::CGTAG_EVENT_Combat_ExecuteDeath,
			DeathPayload
		);
	}
}

const FEnemyPhaseConfig& UCombatPhaseComponent::GetCurrentPhaseConfig() const
{
	if (not PhaseConfigs.IsValidIndex(CurrentPhaseIndex))
		return DefaultPhaseConfig;

	return PhaseConfigs[CurrentPhaseIndex];
}

void UCombatPhaseComponent::InitializePhaseSystem()
{
	AActor* OwnerActor = GetOwner();

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!OwnerASC)
		return;

	OwnerAttributeSet = const_cast<UCombatAttributeSet*>(OwnerASC->GetSet<UCombatAttributeSet>());
	if (!OwnerAttributeSet)
		return;

	if (PhaseConfigs.Num() == 0)
	{
		PhaseConfigs.Add(FEnemyPhaseConfig());
	}

	const float TotalStocks = static_cast<float>(PhaseConfigs.Num());
	OwnerAttributeSet->SetMaxStocks(TotalStocks);
	OwnerAttributeSet->SetRemainingStocks(TotalStocks);

	OwnerASC->GenericGameplayEventCallbacks
		.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_HealthBroken)
		.AddUObject(this, &UCombatPhaseComponent::OnHealthBrokenEvent);

	OwnerASC->GenericGameplayEventCallbacks
		.FindOrAdd(CombatGameplayTags::CGTAG_EVENT_Combat_PostureBroken)
		.AddUObject(this, &UCombatPhaseComponent::OnPostureBrokenEvent);

	ApplyPhaseConfig(0, true);
}

void UCombatPhaseComponent::ApplyPhaseConfig(int32 NewPhaseIndex, bool bIsInitialSetup)
{
	if (!OwnerASC || !PhaseConfigs.IsValidIndex(NewPhaseIndex))
		return;

	if (PhaseConfigs.IsValidIndex(CurrentPhaseIndex))
	{
		const FGameplayTag OldTag = PhaseConfigs[CurrentPhaseIndex].PhaseTag;
		if (OldTag.IsValid())
		{
			OwnerASC->RemoveLooseGameplayTag(OldTag);
		}
		CurrentGrantedHandles.TakeFromAbilitySystem(OwnerASC);
	}

	CurrentPhaseIndex = NewPhaseIndex;
	const FEnemyPhaseConfig& NewConfig = PhaseConfigs[CurrentPhaseIndex];

	if (NewConfig.PhaseTag.IsValid())
	{
		OwnerASC->AddLooseGameplayTag(NewConfig.PhaseTag);
	}

	if (NewConfig.PhaseAbilitySet)
	{
		NewConfig.PhaseAbilitySet->GiveToAbilitySystem(OwnerASC, &CurrentGrantedHandles, GetOwner());
	}

	if (!bIsInitialSetup)
	{
		OnPhaseChanged.Broadcast(CurrentPhaseIndex);
	}
}

void UCombatPhaseComponent::OnHealthBrokenEvent(const FGameplayEventData* Payload)
{
	if (!OwnerASC || !OwnerAttributeSet || OwnerASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Dead))
		return;


	if (Payload && Payload->InstigatorTags.HasTag(CombatGameplayTags::CGTAG_DAMAGE_Type_Deathblow))
	{
		ConsumeStockByDeathblow(*Payload);
		return;
	}

	// 二重発火を阻止
	if (OwnerASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered))
	{
		return;
	}

	const FCombatBreakRule& Rule = GetCurrentPhaseConfig().BreakRule;
	// 残りゲージが２本以上あるか？
	const bool bHasExtraStocks = OwnerAttributeSet->GetRemainingStocks() > 1.0f;

	// 残りゲージが１本以下かつ即死ルールの場合は死亡を確定させる
	if (!bHasExtraStocks && Rule.ZeroHealthBehavior == EZeroHealthBehavior::InstantDeath)
	{
		OwnerAttributeSet->SetRemainingStocks(0.0f);

		FGameplayEventData DeathPayload = Payload ? *Payload : FGameplayEventData();
		DeathPayload.EventTag = CombatGameplayTags::CGTAG_EVENT_Combat_ExecuteDeath;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwner(),
			CombatGameplayTags::CGTAG_EVENT_Combat_ExecuteDeath,
			DeathPayload
		);
		return;
	}

	// ストック２本以上あるか即死ルールではない場合、体幹崩れに移行する
	EnterStaggeredState(ECombatStaggerReason::ZeroHealth, Payload);
}

void UCombatPhaseComponent::OnPostureBrokenEvent(const FGameplayEventData* Payload)
{
	if(!OwnerASC)
		return;

	// 二重発火を阻止
	if (OwnerASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered)
		|| OwnerASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Dead))
	{
		return;
	}

	EnterStaggeredState(ECombatStaggerReason::PostureBreak, Payload);
}

void UCombatPhaseComponent::EnterStaggeredState(ECombatStaggerReason Reason, const FGameplayEventData* Payload)
{
	CurrentStaggerReason = Reason;
	OwnerASC->AddLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered);

	// 体力が０になったのを起因にした場合、体幹崩れを発生させる
	if (Reason == ECombatStaggerReason::ZeroHealth)
	{
		FGameplayEventData StaggerPayload = Payload ? *Payload : FGameplayEventData();
		StaggerPayload.EventTag = CombatGameplayTags::CGTAG_EVENT_Combat_PostureBroken;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwner(),
			CombatGameplayTags::CGTAG_EVENT_Combat_PostureBroken,
			StaggerPayload
		);
	}

	// BreakRuleに従って復帰タイマーを設定
	const FCombatBreakRule& Rule = GetCurrentPhaseConfig().BreakRule;
	const bool bShouldRecover = (Reason == ECombatStaggerReason::ZeroHealth)
		? Rule.bRecoverFromZeroHealth
		: Rule.bRecoverFromPostureBreak;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaggerRecoveryTimerHandle);
		if (bShouldRecover)
		{
			World->GetTimerManager().SetTimer(
				StaggerRecoveryTimerHandle,
				this,
				&UCombatPhaseComponent::OnStaggerRecoveryExpired,
				Rule.StaggerRecoverSeconds,
				false
			);
		}
	}
}

void UCombatPhaseComponent::OnStaggerRecoveryExpired()
{
	if (!OwnerASC || !OwnerAttributeSet)
		return;

	if (CurrentStaggerReason == ECombatStaggerReason::ZeroHealth)
	{
		// 体力を指定された割合だけ回復させる
		const FCombatBreakRule& Rule = GetCurrentPhaseConfig().BreakRule;
		OwnerAttributeSet->SetHealth(OwnerAttributeSet->GetMaxHealth() * Rule.RecoverHealthRatio);

		// 体幹値が最大以上なら８割まで回復させる
		if (OwnerAttributeSet->GetPosture() >= OwnerAttributeSet->GetMaxPosture())
		{
			OwnerAttributeSet->SetPosture(OwnerAttributeSet->GetMaxPosture() * 0.8f);
		}
	}
	else if (CurrentStaggerReason == ECombatStaggerReason::PostureBreak)
	{
		OwnerAttributeSet->SetPosture(OwnerAttributeSet->GetMaxPosture() * 0.8f);
	}

	CurrentStaggerReason = ECombatStaggerReason::None;
	OwnerASC->RemoveLooseGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered);
}
