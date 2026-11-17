// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageResettable.h"
#include "EnemySpawner.generated.h"

UCLASS()
class FASTGAS_DEMO_API AEnemySpawner : public AActor , public IStageResettable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

	// 敵をスポーンさせる
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnEnemy();

	// 保持している敵キャラが生存しているか
	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsEnemyAlive() const;

	virtual void OnStageReset_Implementation() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<ACharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;

	// リセット時に復活するかどうか
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bDoNotRespawnOnceDefeated = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<ACharacter> SpawnedEnemy;

	bool bIsPermanentlyDefeated = false;
};
