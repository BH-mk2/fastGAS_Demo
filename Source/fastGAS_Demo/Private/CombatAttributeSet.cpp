// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "CombatGameplayTags.h"

UCombatAttributeSet::UCombatAttributeSet()
{
	// 初期値の設定
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitPosture(0.0f);
	InitMaxPosture(100.0f);
    InitMoveSpeed(300.0f);
}

void UCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMoveSpeedAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
}

void UCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // 体力が変更された場合、BaseValueも含めて0〜MaxHealthの間に収める
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }

    // 体幹（Posture）の場合も同様に制限
    else if (Data.EvaluatedData.Attribute == GetPostureAttribute())
    {
        const float NewPosture = FMath::Clamp(GetPosture(), 0.0f, GetMaxPosture());
        SetPosture(NewPosture);

        // 体幹崩れの処理を行う
        if (NewPosture >= GetMaxPosture() && GetMaxPosture() > 0.0f)
        {
            UAbilitySystemComponent* ASC = &Data.Target;
            if (!ASC->HasMatchingGameplayTag(CombatGameplayTags::CGTAG_STATE_Action_Staggered))
            {
                FGameplayEventData Payload;
                Payload.Instigator = Data.EffectSpec.GetContext().GetInstigator();
                Payload.Target = ASC->GetAvatarActor();

                UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                    ASC->GetAvatarActor(),
                    CombatGameplayTags::CGTAG_EVENT_Combat_PostureBroken,
                    Payload
                );
            }
        }
    }
}
