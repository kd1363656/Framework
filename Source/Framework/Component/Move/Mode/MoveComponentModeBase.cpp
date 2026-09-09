#include "MoveComponentModeBase.h"

void FWK::MoveComponentModeBase::INIT()
{
	m_canApplyMoveAxisList.clear();

	m_fetchTransformComponentFromSelfGameObjectHelper = {};

	m_rotationDirection = TypeAlias::Math::Vector3::Zero;

	m_canApplyMoveAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);
}

void FWK::MoveComponentModeBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

void FWK::MoveComponentModeBase::PostDeserialize(const GameObject& a_owner)
{
	m_fetchComponentEventObserverFromSelfGameObjectHelper.PostDeserialize(a_owner);
	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize    (a_owner);
}

void FWK::MoveComponentModeBase::Update()
{
}

void FWK::MoveComponentModeBase::EditInspector()
{
}

void FWK::MoveComponentModeBase::ResetMoveDirection()
{
}

nlohmann::json FWK::MoveComponentModeBase::Serialize() const
{
	return nlohmann::json();
}

void FWK::MoveComponentModeBase::AddCanApplyMoveAxis(const Enum::Axis a_canApplyMoveAxis)
{

}