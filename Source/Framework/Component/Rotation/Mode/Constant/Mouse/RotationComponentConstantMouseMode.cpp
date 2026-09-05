#include "RotationComponentConstantMouseMode.h"

void FWK::RotationComponentConstantMouseMode::INIT()
{
	m_inspector = {};

	m_jsonConverter = {};
}

void FWK::RotationComponentConstantMouseMode::Update()
{
	if (!CanUpdate()) { return; }

	const auto& l_inputManager    = InputManager::GetInstance           ();
	const auto& l_mouseController = l_inputManager.GetREFMouseController();
	const auto& l_mouseMovement   = l_mouseController.GetREFMovement    ();

	// マウスの移動量がほとんどないなら"return"
	if (l_mouseMovement.LengthSquared() <= std::numeric_limits<float>::epsilon()) { return; }
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