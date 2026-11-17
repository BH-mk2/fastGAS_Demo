// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckPointActor.generated.h"

class USphereComponent;
class UArrowComponent;

UCLASS()
class FASTGAS_DEMO_API ACheckPointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckPointActor();

	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void ActivateCheckpoint(APawn* PlayerPawn);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> RespawnPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Checkpoint")
	TSubclassOf<class UGameplayEffect> RestorePlayerGE;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
