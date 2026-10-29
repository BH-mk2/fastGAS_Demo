#include "CombatGameplayTags.h"

namespace CombatGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_Input_Attack, "Event.Input.Attack");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_Input_GuardEnd, "Event.Input.GuardEnd");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_Input_Deathblow, "Event.Input.Deathblow");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_Input_Evaluate, "Event.Input.Evaluate");

	// StateTreeイベント用
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_State_Cancellable_Attack, "Event.State.Cancellable.Attack"); // 攻撃キャンセル可能を通知

	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action, "State.Action");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action_Attack, "State.Action.Attack");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action_Guard, "State.Action.Guard");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action_Parry, "State.Action.Parry");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action_Dodge, "State.Action.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action_Staggered, "State.Action.Staggered");

	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Combat_LockedOn, "State.Combat.LockedOn");

	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Combat_Cancellable_Attack, "State.Combat.Cancellable.Attack");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Combat_Cancellable_Dodge, "State.Combat.Cancellable.Dodge");


	UE_DEFINE_GAMEPLAY_TAG(CGTAG_DATA_Damage_Health, "Data.Damage.Health");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_DATA_Damage_Posture, "Data.Damage.Posture");

	/* Gameplay Cue用 */
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_GAMEPLAYCUE_Hit_Direct, "GameplayCue.Hit.Direct");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_GAMEPLAYCUE_Hit_Guard, "GameplayCue.Hit.Guard");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_GAMEPLAYCUE_Hit_Parry, "GameplayCue.Hit.Parry");
}
