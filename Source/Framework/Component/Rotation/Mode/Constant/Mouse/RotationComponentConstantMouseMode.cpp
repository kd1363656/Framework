#include "RotationComponentConstantMouseMode.h"

void FWK::RotationComponentConstantMouseMode::INIT()
{
	m_inspector = {};

	m_jsonConverter = {};
}

void FWK::RotationComponentConstantMouseMode::Update()
{
	auto l_fetchTransformComponentFromSelfGameObjectHelper = GetREFFetchTransformComponentFromSelfGameObjectHelper                            ();
	auto l_fetchTransformComponent                         = l_fetchTransformComponentFromSelfGameObjectHelper.GetREFFetchedTransformComponent().lock();

	if (!l_fetchTransformComponent) { return; }

	const auto& l_inuputManager   = InputManager::GetInstance            ();
	const auto& l_mouseController = l_inuputManager.GetREFMouseController();


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