// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "EnemyPhaseTypes.h"
#include "CombatAbilitySet.h"
#include "CombatPhaseComponent.generated.h"

class UAbilitySystemComponent;
class UCombatAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCOmbatPhaseChangedSignature, int32, NewPhaseIndex);

UENUM(BlueprintType)
enum class ECombatStaggerReason : uint8
{
	None,
	PostureBreak,
	ZeroHealth,
};

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class FASTGAS_DEMO_API UCombatPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatPhaseComponent();

	FGameplayTagContainer GetCurrentAllowedAttackTags();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Phase")
	void ConsumeStockByDeathblow(const FGameplayEventData& DeathblowPayload);

	UFUNCTION(BlueprintPure, Category = "Combat|Phase")
	const FEnemyPhaseConfig& GetCurrentPhaseConfig() const;

	UFUNCTION(BlueprintPure, Category = "Combat|Phase")
	int32 GetCurrentPhaseIndex() const { return CurrentPhaseIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Combat|Phase")
	FOnCOmbatPhaseChangedSignature OnPhaseChanged;

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Phase")
	TArray<FEnemyPhaseConfig> PhaseConfigs;

	UPROPERTY(Transient)
	FCombatAbilitySet_GrantedHandles CurrentGrantedHandles;

private:
	void InitializePhaseSystem();
	void ApplyPhaseConfig(int32 NewPhaseIndex, bool bIsInitialSetup);

	void OnHealthBrokenEvent(const FGameplayEventData* Payload);
	void OnPostureBrokenEvent(const FGameplayEventData* Payload);

	void EnterStaggeredState(ECombatStaggerReason Reason, const FGameplayEventData* Payload);
	void OnStaggerRecoveryExpired();

	void AdvanceToNextPhase();
	UFUNCTION()
	void OnIncapacitatedTagChanged(FGameplayTag Tag, int32 NewCount);
	FGameplayEventData CachedDeathblowPayload;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC;

	UPROPERTY()
	TObjectPtr<UCombatAttributeSet> OwnerAttributeSet;

	int32 CurrentPhaseIndex = 0;
	ECombatStaggerReason CurrentStaggerReason = ECombatStaggerReason::None;
	FTimerHandle StaggerRecoveryTimerHandle;

	static const FEnemyPhaseConfig DefaultPhaseConfig;
};
