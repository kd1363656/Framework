#include "RotationComponentInspector.h"

void FWK::RotationComponentInspector::EditInspector(RotationComponent& a_moveComponent)
{
    auto& l_rotationMode = a_moveComponent.GetMutableREFRotationMode();

    // ラジオボタンから回転方法を選択する
    Utility::IMGUIFactoryRadioButtonSelector<TypeAlias::RotatationComponentModeUniqueFactory>(k_rotationModeRadioButtonSelectorLabel, l_rotationMode);

    if (l_rotationMode)
    {
        l_rotationMode->EditInspector();
    }
}