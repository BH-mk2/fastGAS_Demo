// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacterBase.h"
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
}

void ACombatCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeDefaultAttributes();
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

void ACombatCharacterBase::InitializeDefaultAttributes()
{
	if (!AbilitySystemComponent)
		return;

	if (!DefaultAttricutesEffect)
		return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttricutesEffect, 1.0f, EffectContext);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	}
}
