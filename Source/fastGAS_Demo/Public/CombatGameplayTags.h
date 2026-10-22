#pragma once

#include "NativeGameplayTags.h"

namespace CombatGameplayTags
{
	// イベント用
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CGTAG_EVENT_Input_Attack); // 攻撃の入力
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CGTAG_EVENT_Input_Evaluate); // 入力を評価(先行入力用)

	// 状態用
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CGTAG_STATE_Action);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CGTAG_STATE_Action_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CGTAG_STATE_Combat_Cancellable_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CGTAG_STATE_Combat_Cancellable_Dodge);
}
