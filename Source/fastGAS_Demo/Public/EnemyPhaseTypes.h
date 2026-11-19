#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnemyPhaseTypes.generated.h"

class UCombatAbilitySet;

UENUM(BlueprintType)
enum class EZeroHealthBehavior : uint8
{
	InstantDeath,
	EnterDeathblowState
};

USTRUCT(BlueprintType)
struct FCombatBreakRule
{
	GENERATED_BODY()

	// 体力が０になった時の挙動
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BreakRule")
	EZeroHealthBehavior ZeroHealthBehavior = EZeroHealthBehavior::InstantDeath;

	// 体力０による忍殺待機児、時間経過で復帰するか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BreakRule")
	bool bRecoverFromZeroHealth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BreakRule")
	bool bRecoverFromPostureBreak = true;

	// 時間経過で復帰する場合の猶予時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BreakRule", Meta = (ClampMin = "0.5"))
	float StaggerRecoverSeconds = 4.0f;

	// 体力０から復帰した際に回復する体力の割合
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BreakRule", Meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float RecoverHealthRatio = 0.25f;
};

USTRUCT(BlueprintType)
struct FEnemyPhaseConfig
{
	GENERATED_BODY()

	// このフェーズ中にASCへ付与するタグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FGameplayTag PhaseTag;

	// このフェーズ固有の行動をまとめたアビリティ&GameplayEffectセット
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Combat")
	TObjectPtr<UCombatAbilitySet> PhaseAbilitySet;

	// このフェーズでAIが抽選・使用できる攻撃アビリティのタグ一覧
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Combat")
	FGameplayTagContainer AllowedAttackTags;

	// このフェーズにおける体力０・体幹崩れ時の挙動ルール
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Break")
	FCombatBreakRule BreakRule;
};
