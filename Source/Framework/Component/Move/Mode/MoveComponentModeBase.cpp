#include "MoveComponentModeBase.h"

void FWK::MoveComponentModeBase::INIT()
{
	m_canApplyMoveAxisList.clear();

	m_fetchTransformComponentFromSelfGameObjectHelper = {};

	m_moveDirection = TypeAlias::Math::Vector3::Zero;

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
	m_moveDirection = TypeAlias::Math::Vector3::Zero;
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
	// 同じ要素を含めない
	if (std::ranges::any_of(m_canApplyMoveAxisList, 
		                   [a_canApplyMoveAxis](const auto a_containsApplyMoveAxis) 
 	 	                   {
								return a_containsApplyMoveAxis == a_canApplyMoveAxis;
		                   }))
	{
		return;
	}

	// jsonに保存してもビットシフトの値が変わっても問題ないように
	// Enumをstd::vectorで保存する
	m_canApplyMoveAxisList.emplace_back(a_canApplyMoveAxis);

	// ビットフラグを反映する
	m_canApplyMoveAxis = Utility::EnableFlag(a_canApplyMoveAxis, m_canApplyMoveAxis);
}