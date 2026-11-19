// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacterBase.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "CombatAttributeSet.h"
#include "WeaponBase.h"

// Sets default values
ACombatCharacterBase::ACombatCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("AttributeSet"));
}

bool ACombatCharacterBase::IsAbilityActiveByTag(FGameplayTag AbilityTag) const
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid())
		return false;

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->GetAssetTags().HasTag(AbilityTag))
		{
			return true;
		}
	}
	return false;
}

void ACombatCharacterBase::TriggerAction(FGameplayTag AbilityTag, FGameplayTag FollowUpEventTag)
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid())
		return;

	// 起動中であれば、追加入力を送る
	if (IsAbilityActiveByTag(AbilityTag))
	{
		if (FollowUpEventTag.IsValid())
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				this,
				FollowUpEventTag,
				FGameplayEventData()
			);
		}
		return;
	}

	// 未起動なら新規起動する
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
}

void ACombatCharacterBase::ReleaseAction(FGameplayTag AbilityTag, FGameplayTag ReleaseEventTag)
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid() || !ReleaseEventTag.IsValid())
	{
		return;
	}

	if (IsAbilityActiveByTag(AbilityTag))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, ReleaseEventTag, FGameplayEventData());
	}
}

void ACombatCharacterBase::ResetAttributes()
{
	if (AbilitySystemComponent && DefaultAbilitySet)
	{
		DefaultAbilitySet->ApplyGameplayEffectsToAbilitySystem(AbilitySystemComponent, &DefaultGrantedHandles, this);
	}
}

// Called when the game starts or when spawned
void ACombatCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (!WeaponClass)
	{
		ensure(WeaponClass!=nullptr);
		return;
	}

	Weapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
	Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachToSocketName);

	if (AbilitySystemComponent && AttributeSet && DefaultAbilitySet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UCombatAttributeSet::GetMoveSpeedAttribute()
		).AddUObject(this, &ACombatCharacterBase::OnMoveSpeedChanged);

		if (UCharacterMovementComponent* CMC = GetCharacterMovement())
		{
			CMC->MaxWalkSpeed = AttributeSet->GetMoveSpeed();
		}

		DefaultAbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &DefaultGrantedHandles, this);
	}
}

void ACombatCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ACombatCharacterBase::ResetHitActors()
{
	HitActors.Reset();
}

bool ACombatCharacterBase::AddHitActor(TWeakObjectPtr<AActor> HitActor)
{
	if (!HitActor.IsValid())
	{
		return false;
	}
	if (HitActors.Contains(HitActor))
	{
		return false;
	}

	HitActors.Add(HitActor);

	return true;
}

// Called every frame
void ACombatCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACombatCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACombatCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->MaxWalkSpeed = Data.NewValue;
	}
}
