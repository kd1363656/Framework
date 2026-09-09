#include "RotationComponentModeBaseInspector.h"

void FWK::RotationComponentModeBaseInspector::EditInspector(RotationComponentModeBase& a_moveComponentModeBase)
{
	auto& l_canApplyRotationAxisList = a_moveComponentModeBase.GetMutableREFCanApplyRotationAxisList();

	Utility::IMGUIStringValueBidirectionalRegistryCheckBoxSelector(k_rotationApplyAxisCheckboxLabel, l_canApplyRotationAxisList);
}