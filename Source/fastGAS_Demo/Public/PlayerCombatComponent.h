// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "PlayerCombatComponent.generated.h"

class ASamuraiCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FASTGAS_DEMO_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|LockOn")
	void ToggleLockOnTarget();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float LockOnRadius = 1500.0f;

private:
	TWeakObjectPtr<AActor> SearchTarget();

	void OnPlayerHealthBroken(const FGameplayEventData* Payload);

	UPROPERTY()
	TObjectPtr<ASamuraiCharacter> OwnerSamurai;
};
