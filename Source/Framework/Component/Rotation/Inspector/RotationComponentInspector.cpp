#include "RotationComponentInspector.h"

void FWK::RotationComponentInspector::EditInspector(RotationComponent& a_rotationComponent)
{
    auto l_rotationMode = a_rotationComponent.GetVALRotationMode().lock();

    // ラジオボタンから回転方法を選択する
    Utility::IMGUIFactoryRadioButtonSelector<TypeAlias::RotationComponentModeSharedFactory>(k_rotationModeRadioButtonSelectorLabel, l_rotationMode);

    if (l_rotationMode)
    {
        l_rotationMode->EditInspector();
    }
}