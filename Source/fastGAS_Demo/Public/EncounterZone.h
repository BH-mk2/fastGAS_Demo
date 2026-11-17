// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageResettable.h"
#include "EncounterZone.generated.h"

class UBoxComponent;
class AEnemySpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEncountaerStateChangedSignature);

UCLASS()
class FASTGAS_DEMO_API AEncounterZone : public AActor, public IStageResettable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEncounterZone();

	virtual void OnStageReset_Implementation() override;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncountaerStateChangedSignature OnEncounterStarted;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncountaerStateChangedSignature OnEncounterCleared;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// エリア侵入検知用トリガー
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// 敵スポナー群
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter")
	TArray<TObjectPtr<AEnemySpawner>> TargetSpawners;

	// エリアを封鎖するアクター群
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter")
	TArray<TObjectPtr<AActor>> BarrierActors;

	// 一度クリアしたらリトライしても再度発生しない場合はtrue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	bool bIsOneTimeOnly = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	void StartEncounter();
	void ClearEncounter();
	void CheckEncounterProgress();
	void SetBarriersActive(bool bActive);

	bool bIsActive = false;
	bool bIsCleared = false;
	FTimerHandle CheckTimerHandle;
};
