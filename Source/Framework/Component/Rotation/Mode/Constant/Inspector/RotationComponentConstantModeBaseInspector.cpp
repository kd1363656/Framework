#include "RotationComponentConstantModeBaseInspector.h"

void FWK::RotationComponentConstantModeBaseInspector::EditInspector(RotationComponentConstantModeBase& a_moveComponentConstantModeBase)
{
	a_moveComponentConstantModeBase.RotationComponentModeBase::EditInspector();

 	float l_rotationSpeed = a_moveComponentConstantModeBase.GetVALRotationSpeed();

	if (ImGui::DragFloat(k_rotationSpeedLabel.data(), &l_rotationSpeed, Constant::k_imguiDefaultDragValue))
	{
		a_moveComponentConstantModeBase.SetRotationSpeed(l_rotationSpeed);
	}
}