#include "RotationComponent.h"

void FWK::RotationComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::RotationComponent::Update()
{
	if (!m_rotationMode) { return; }

	const auto& l_gameObject = GetREFOwner().lock();

	if (!l_gameObject) { return; }

	const auto& l_componentEventObserver = l_gameObject->GetVALComponentEventObserver().lock();

	if (!l_componentEventObserver) { return; }

	// イベントから回転できるかどうかを取得する
	// できないなら書いて処理を行わない
	if (!l_componentEventObserver->IsEventMatching(Enum::ComponentEvent::CanRotation, Enum::EventLane::TriggeredKeepFrame)) { return; }

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