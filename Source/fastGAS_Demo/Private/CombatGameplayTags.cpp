#include "CombatGameplayTags.h"

namespace CombatGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_Input_Attack, "Event.Input.Attack");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_EVENT_Input_Evaluate, "Event.Input.Evaluate");

	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action, "State.Action");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Action_Attack, "State.Action.Attack");

	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Combat_Cancellable_Attack, "State.Combat.Cancellable.Attack");
	UE_DEFINE_GAMEPLAY_TAG(CGTAG_STATE_Combat_Cancellable_Dodge, "State.Combat.Cancellable.Dodge");
}
