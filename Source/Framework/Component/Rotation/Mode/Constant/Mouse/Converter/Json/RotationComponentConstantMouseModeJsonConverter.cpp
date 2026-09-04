#include "RotationComponentConstantMouseModeJsonConverter.h"

void FWK::Converter::RotationComponentConstantMouseModeJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode) const
{
	if (a_rootJson.is_null()) { return; }

	a_rotationComponentConstantMouseMode.RotationComponentConstantModeBase::Deserialize(a_rootJson);

	Struct::Range<float> m_xAxisRotatableRange = { a_rootJson.value(k_xAxisRotatableRangeMAXJsonKey, Constant::k_rotationComponentDefaultRotatableRangeX), a_rootJson.value(k_xAxisRotatableRangeMINJsonKey, -Constant::k_rotationComponentDefaultRotatableRangeX) };

	a_rotationComponentConstantMouseMode.SetXAxisRotatableRange(m_xAxisRotatableRange);
}

nlohmann::json FWK::Converter::RotationComponentConstantMouseModeJsonConverter::Serialize(const RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode) const
{
	auto l_rootJson = a_rotationComponentConstantMouseMode.RotationComponentConstantModeBase::Serialize();

	const auto& l_xAxisRotatableRange = a_rotationComponentConstantMouseMode.GetREFXAxisRotatableRange();

	l_rootJson[k_xAxisRotatableRangeMAXJsonKey] = l_xAxisRotatableRange.m_max;
	l_rootJson[k_xAxisRotatableRangeMINJsonKey] = l_xAxisRotatableRange.m_min;

	return l_rootJson;
}