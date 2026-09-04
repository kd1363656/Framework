#include "RotationComponentInspector.h"

void FWK::RotationComponentInspector::EditInspector(RotationComponent& a_rotationComponent)
{
	auto& l_rotationMode = a_rotationComponent.GetMutableREFRotationMode();

	// ラジオボタンから回転方法を選択する
	Utility::FactoryRadioButtonSelector<TypeAlias::RotatationComponentModeUniqueFactory>(k_rotationModeRadioButtonSelectorLabel, l_rotationMode);

	if (l_rotationMode)
	{
		l_rotationMode->EditInspector();
	}
}