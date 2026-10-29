// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "WeaponBase.h"
#include "CombatGameplayTags.h"

AEnemyCharacter::AEnemyCharacter()
{
}

void AEnemyCharacter::ExecuteAttack()
{
	if (!AbilitySystemComponent->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Attack))
	{
		AbilitySystemComponent->TryActivateAbilityByClass(AttackAbilityClass);
	}
	else
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CombatGameplayTags::CGTAG_EVENT_Input_Attack, FGameplayEventData{});
	}
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AttackAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbilityClass,1));

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Combat_Cancellable_Attack)
		.AddUObject(this, &AEnemyCharacter::OnAttackCancellableInner);

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Attack)
		.AddUObject(this, &AEnemyCharacter::OnAttackEndInner);
}

void AEnemyCharacter::OnAttackCancellableInner(FGameplayTag Tag, int32 count)
{
	if (count != 0)
	{
		OnAttackCancellable.Broadcast();
	}
}

void AEnemyCharacter::OnAttackEndInner(FGameplayTag Tag, int32 count)
{
	if (count == 0)
	{
		OnAttackEnd.Broadcast();
	}
}

bool AEnemyCharacter::GetAttackPoints(FVector& OutStart, FVector& OutEnd)const
{
	if (!Weapon)
		return false;
	auto WeaponMesh = Weapon->GetWeaponMesh();
	if (!WeaponMesh)
		return false;

	OutStart = WeaponMesh->GetSocketLocation("WeaponStart");
	OutEnd = WeaponMesh->GetSocketLocation("WeaponEnd");

	return true;
}
