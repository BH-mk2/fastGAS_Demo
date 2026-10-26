// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAttributeSet.h"
#include "GameplayEffectExtension.h"

UCombatAttributeSet::UCombatAttributeSet()
{
	// 初期値の設定
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitPosture(0.0f);
	InitMaxPosture(100.0f);
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
        SetPosture(FMath::Clamp(GetPosture(), 0.0f, GetMaxPosture()));
    }
}
