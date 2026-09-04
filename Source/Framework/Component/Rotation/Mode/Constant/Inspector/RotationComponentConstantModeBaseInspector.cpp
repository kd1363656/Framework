#include "RotationComponentConstantModeBaseInspector.h"

void FWK::RotationComponentConstantModeBaseInspector::EditInspector(RotationComponentConstantModeBase& a_rotationComponentConstantModeBase)
{
	a_rotationComponentConstantModeBase.RotationComponentModeBase::EditInspector();

 	float l_rotationSpeed = a_rotationComponentConstantModeBase.GetVALRotationSpeed();

	if (ImGui::DragFloat(k_rotationSpeedLabel.data(), &l_rotationSpeed, Constant::k_imguiDefaultDragValue))
	{
		a_rotationComponentConstantModeBase.SetRotationSpeed(l_rotationSpeed);
	}
}