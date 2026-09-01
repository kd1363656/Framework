#include "RotationComponentModeBaseInspector.h"

void FWK::RotationComponentModeBaseInspector::EditInspector(RotationComponentModeBase& a_rotationComponentModeBase)
{
	auto& l_rotationApplyAxisList = a_rotationComponentModeBase.GetMutableREFRotationApplyAxisList();

	Utility::StringValueBidirectionalRegistryCheckBoxSelector(k_rotationApplyAxisCheckboxLabel, l_rotationApplyAxisList);
}