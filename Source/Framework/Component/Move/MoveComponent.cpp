#include "MoveComponent.h"

void FWK::MoveComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::MoveComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}

nlohmann::json FWK::MoveComponent::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}