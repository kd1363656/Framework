#include "MoveComponent.h"

void FWK::MoveComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::MoveComponent::PostDeserialize()
{
    if (!m_moveMode) { return; }

    const auto& l_owner = GetREFOwner().lock();

    if (!l_owner) { return; }

    m_moveMode->PostDeserialize(*l_owner);
}

void FWK::MoveComponent::Update()
{
    if (!m_moveMode) { return; }

    m_moveMode->Update();
}

void FWK::MoveComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::MoveComponent::SerializePrefab()
{
    return m_jsonConverter.SerializePrefab(*this);
}