#include "RotationComponentConstantMouseModeInspector.h"

void FWK::RotationComponentConstantMouseModeInspector::EditInspector(RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode)
{
	a_rotationComponentConstantMouseMode.RotationComponentConstantModeBase::EditInspector();

	auto& l_xAxisRotatableRange = a_rotationComponentConstantMouseMode.GetMutableREFXAxisRotatableRange();

	ImGui::SeparatorText(k_xAxisRotatable.data());

	ImGui::DragFloat(k_xAxisRotatableMAX.data(), &l_xAxisRotatableRange.m_max, Constant::k_imguiDefaultDragValue);
	ImGui::DragFloat(k_xAxisRotatableMIN.data(), &l_xAxisRotatableRange.m_min, Constant::k_imguiDefaultDragValue);
}