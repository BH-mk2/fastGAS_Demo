#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"
#include "../Public/CombatCharacterBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatCharacterBaseCheckTest,                                      // テストクラス名
    "CombatSystem.CharacterBase.AllCharactersImplementDirection",       // UI上に表示されるツリー階層名
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter // 実行フラグ
)

bool FCombatCharacterBaseCheckTest::RunTest(const FString& Parameters)
{
    TArray<UClass*> DerivedClasses;
    GetDerivedClasses(ACombatCharacterBase::StaticClass(), DerivedClasses, true);

    for (UClass* Class : DerivedClasses)
    {
        if (Class->HasAnyClassFlags(CLASS_Abstract))
            continue;

        ACombatCharacterBase* CDO = Class->GetDefaultObject<ACombatCharacterBase>();
        TestNotNull(FString::Printf(TEXT("CDO must exist for %s"), *Class->GetName()), CDO);
        auto tmp = CDO->GetDesiredMoveDirection();
    }
    return true;
}
