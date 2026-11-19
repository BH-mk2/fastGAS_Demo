// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "CombatAbilitySet.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FCombatAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;
};

USTRUCT(BlueprintType)
struct FCombatAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float EffectLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct FCombatAbilitySet_GrantedHandles
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void TakeFromAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
};

UCLASS(BlueprintType)
class FASTGAS_DEMO_API UCombatAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, FCombatAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

	void ApplyGameplayEffectsToAbilitySystem(UAbilitySystemComponent* ASC, FCombatAbilitySet_GrantedHandles* OutGrantedHandles = nullptr, UObject* SourceObject = nullptr) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Abilities", meta = (TitleProperty = Ability))
	TArray<FCombatAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Effects", meta = (TitleProperty = GameplayEffect))
	TArray<FCombatAbilitySet_GameplayEffect> GrantedGameplayEffects;
};
