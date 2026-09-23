#include "MoveComponentInspector.h"

void FWK::MoveComponentInspector::EditInspector(MoveComponent& a_moveComponent)
{
    auto l_moveMode = a_moveComponent.GetVALMoveMode().lock();

    // ラジオボタンから回転方法を選択する
    Utility::IMGUIFactoryRadioButtonSelector<TypeAlias::MoveComponentModeSharedFactory>(k_moveModeRadioButtonSelectorLabel, l_moveMode);

    if (l_moveMode)
    {
        l_moveMode->EditInspector();
    }
}