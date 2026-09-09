#include "RotationComponent.h"

void FWK::RotationComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::RotationComponent::PostDeserialize()
{
	if (!m_rotationMode) { return; }

	const auto& l_owner = GetREFOwner().lock();

	if (!l_owner) { return; }

	m_rotationMode->PostDeserialize(*l_owner);
}

void FWK::RotationComponent::Update()
{
	if (!m_rotationMode) { return; }

	m_rotationMode->Update();
}

void FWK::RotationComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}

nlohmann::json FWK::RotationComponent::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}