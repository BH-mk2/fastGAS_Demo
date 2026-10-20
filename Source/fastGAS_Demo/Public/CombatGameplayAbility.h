// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CombatCharacterBase.h"
#include "CombatGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class FASTGAS_DEMO_API UCombatGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	ACombatCharacterBase* GetCombatCharacter();
};
