// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "WeaponBase.h"

AEnemyCharacter::AEnemyCharacter()
{

}

void AEnemyCharacter::ExecuteAttack()
{
	const bool Result = AbilitySystemComponent->TryActivateAbilityByClass(AttackAbilityClass);
	UE_LOG(LogTemp, Warning, TEXT("EnemyAttackTry:%s"), Result ? TEXT("true") : TEXT("false"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AttackAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbilityClass,1));
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
