#include "STE_CombatContext.h"

#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"

#include "SamuraiCharacter.h"
#include "CombatPhaseComponent.h"
#include "CombatAttributeSet.h"

void FSTE_CombatContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UpdateContextData(InstanceData);

	if (InstanceData.OwnerCharacter)
	{
		if (auto ASC = InstanceData.OwnerCharacter->GetAbilitySystemComponent())
		{
			ASC->GetOwnedGameplayTags(InstanceData.OwnedTags);
		}
	}
}

void FSTE_CombatContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UpdateContextData(InstanceData);
}

void FSTE_CombatContext::UpdateContextData(FInstanceDataType& InstanceData) const
{
	ASamuraiCharacter* Character = InstanceData.OwnerCharacter;
	if (!Character)
		return;

	TWeakObjectPtr<AActor> Target = Character->GetLockedOnTarget();
	if (not Target.IsValid())
	{
		InstanceData.TargetActor = nullptr;
		InstanceData.AngleToTarget = 0.0f;
		InstanceData.DistanceToTarget = TNumericLimits<float>::Max();
	}
	else
	{
		InstanceData.TargetActor = Target.Get();

		FVector ToTarget = Target->GetActorLocation() - Character->GetActorLocation();
		ToTarget.Z = 0.0f;
		InstanceData.DistanceToTarget= ToTarget.Size();

		if (ToTarget.IsNearlyZero())
		{
			InstanceData.AngleToTarget = 0.0f;
		}
		else
		{
			const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
			const FVector Dir = ToTarget.GetSafeNormal();
			const float Dot = FMath::Clamp(FVector::DotProduct(Forward, Dir), -1.0f, 1.0f); // ab * cosθ
			const float Radian = FMath::Acos(Dot);
			InstanceData.AngleToTarget = FMath::RadiansToDegrees(Radian);
		}
	}

	if (UCombatPhaseComponent* PhaseComp = Character->FindComponentByClass<UCombatPhaseComponent>())
	{
		InstanceData.AllowedAttackTags = PhaseComp->GetCurrentAllowedAttackTags();
	}
	else
	{
		InstanceData.AllowedAttackTags.Reset();
	}

	if (const UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		if (const UCombatAttributeSet* AttrSet = ASC->GetSet<UCombatAttributeSet>())
		{
			const float MaxHP = AttrSet->GetMaxHealth();
			InstanceData.HealthRatio = (MaxHP > 0.0f) ? (AttrSet->GetHealth() / MaxHP) : 0.0f;

			const float MaxPosture = AttrSet->GetMaxPosture();
			InstanceData.PostureRatio = (MaxPosture > 0.0f) ? (AttrSet->GetPosture() / MaxPosture) : 0.0f;
		}

		ASC->GetOwnedGameplayTags(InstanceData.OwnedTags);
	}
}
