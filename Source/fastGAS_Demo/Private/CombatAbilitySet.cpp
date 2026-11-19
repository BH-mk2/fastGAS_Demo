// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

void FCombatAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FCombatAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC)
		return;

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}
	AbilitySpecHandles.Reset();
}

void UCombatAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, FCombatAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	if (!ASC)
		return;

	for (const FCombatAbilitySet_GameplayAbility& AbilityToGrant : GrantedGameplayAbilities)
	{
		if (not IsValid(AbilityToGrant.Ability))
			continue;

		FGameplayAbilitySpec AbilitySpec(AbilityToGrant.Ability, AbilityToGrant.AbilityLevel, INDEX_NONE, SourceObject);
		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Handle);
		}
	}

	ApplyGameplayEffectsToAbilitySystem(ASC, OutGrantedHandles, SourceObject);
}

void UCombatAbilitySet::ApplyGameplayEffectsToAbilitySystem(UAbilitySystemComponent* ASC, FCombatAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	if (!ASC)
		return;

	for (const FCombatAbilitySet_GameplayEffect& EffectToGrant : GrantedGameplayEffects)
	{
		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(SourceObject);
		const FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectToSelf(
			GameplayEffect,
			EffectToGrant.EffectLevel,
			Context
		);
	}
}
