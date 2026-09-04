#include "RotationComponentConstantMouseMode.h"

void FWK::RotationComponentConstantMouseMode::INIT()
{
	m_inspector = {};

	m_jsonConverter = {};
}

void FWK::RotationComponentConstantMouseMode::Update()
{

}

void FWK::RotationComponentConstantMouseMode::EditInspector()
{
	m_inspector.EditInspector(*this);
}

void FWK::RotationComponentConstantMouseMode::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::RotationComponentConstantMouseMode::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}