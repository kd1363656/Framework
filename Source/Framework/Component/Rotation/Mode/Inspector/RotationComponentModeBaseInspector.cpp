#include "RotationComponentModeBaseInspector.h"

void FWK::RotationComponentModeBaseInspector::EditInspector(RotationComponentModeBase& a_rotationComponentModeBase)
{
    auto& l_canApplyRotationAxisBitShiftFlagList = a_rotationComponentModeBase.GetMutableREFCanApplyRotationAxisBitShiftFlagList();

    Utility::IMGUIStringValueBidirectionalRegistryCheckBoxSelector(k_rotationApplyAxisBitShiftFlagCheckboxLabel, l_canApplyRotationAxisBitShiftFlagList);
}