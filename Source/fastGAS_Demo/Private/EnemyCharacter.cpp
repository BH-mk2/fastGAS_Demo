// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "WeaponBase.h"
#include "CombatAttributeSet.h"
#include "CombatGameplayTags.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h" // CharacterのCapsuleComponent取得に必要
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

AEnemyCharacter::AEnemyCharacter()
{
	WidgetBars = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetBars"));
	WidgetBars->SetupAttachment(GetCapsuleComponent());
	WidgetBars->SetMobility(EComponentMobility::Movable);
	WidgetTargetMarker = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetTargetMarker"));
	WidgetTargetMarker->SetupAttachment(GetCapsuleComponent());
	WidgetTargetMarker->SetMobility(EComponentMobility::Movable);
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

void AEnemyCharacter::ExecuteGuard()
{
	AbilitySystemComponent->TryActivateAbilityByClass(GuardAbilityClass);
}

void AEnemyCharacter::ExecuteGuardEnd()
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CombatGameplayTags::CGTAG_EVENT_Input_GuardEnd, FGameplayEventData{});
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AttackAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbilityClass,1));

	if (!GuardAbilityClass)
		return;

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GuardAbilityClass, 1));

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Combat_Cancellable_Attack)
		.AddUObject(this, &AEnemyCharacter::OnAttackCancellableInner);

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Attack)
		.AddUObject(this, &AEnemyCharacter::OnAttackEndInner);

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Guard)
		.AddUObject(this, &AEnemyCharacter::OnGuardEndInner);

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Combat_LockedOn)
		.AddUObject(this, &AEnemyCharacter::OnLockChangedInner);

	AbilitySystemComponent->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Staggered)
		.AddUObject(this, &AEnemyCharacter::OnStaggerChangedInner);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UCombatAttributeSet::GetHealthAttribute()
	).AddUObject(this, &AEnemyCharacter::OnHealthChangedInner);
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

void AEnemyCharacter::OnGuardEndInner(FGameplayTag Tag, int32 count)
{
	if (count == 0)
	{
		OnGuardEnd.Broadcast();
	}
}

void AEnemyCharacter::OnLockChangedInner(FGameplayTag Tag, int32 count)
{
	bool IsLocked = count != 0;
	OnLockChanged.Broadcast(IsLocked);
}

void AEnemyCharacter::OnStaggerChangedInner(FGameplayTag Tag, int32 count)
{
	bool IsStagger = count != 0;
	OnStaggerChanged.Broadcast(IsStagger);
	if (IsStagger)
	{
		const FGameplayTagContainer WithoutTags(CombatGameplayTags::CGTAG_STATE_Action_Staggered);
		AbilitySystemComponent->CancelAbilities(nullptr, &WithoutTags);
		PlayAnimMontage(StaggerMontage);
	}
}

void AEnemyCharacter::OnHealthChangedInner(const FOnAttributeChangeData& Data)
{
	if (Data.OldValue > 0.0f && Data.NewValue <= 0.0f)
	{
		// 死亡時
		AbilitySystemComponent->CancelAbilities();
		PlayAnimMontage(DeathMontage);

		OnDeath.Broadcast();
	}
	else if (Data.OldValue <= 0.0f && Data.NewValue > 0.0f)
	{
		// 復活時
		OnRevival.Broadcast();
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
