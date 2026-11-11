#pragma once

#include "CoreMinimal.h"
#include "CombatAbilityInputID.generated.h"

UENUM(BlueprintType)
enum class ECombatAbilityInputID : uint8
{
	None = 0,
	Guard = 1,
};
