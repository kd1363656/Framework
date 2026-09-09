#include "RotationComponentModeBaseInspector.h"

void FWK::RotationComponentModeBaseInspector::EditInspector(RotationComponentModeBase& a_rotationComponentModeBase)
{
	auto& l_canApplyRotationAxisList = a_rotationComponentModeBase.GetMutableREFCanApplyRotationAxisList();

	Utility::IMGUIStringValueBidirectionalRegistryCheckBoxSelector(k_rotationApplyAxisCheckboxLabel, l_canApplyRotationAxisList);
}