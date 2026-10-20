// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatGameplayAbility.h"

ACombatCharacterBase* UCombatGameplayAbility::GetCombatCharacter()
{
	if (!CurrentActorInfo)
		return nullptr;

	auto ret = Cast<ACombatCharacterBase>(CurrentActorInfo->AvatarActor);
	return ret;
}
