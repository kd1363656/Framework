#include "RotationComponentModeBase.h"

void FWK::RotationComponentModeBase::INIT()
{
	m_canApplyRotationAxisList.clear();

	m_fetchTransformComponentFromSelfGameObjectHelper = {};

	m_inspector = {};

	m_jsonConverter = {};

	m_rotationDirection = TypeAlias::Math::Vector3::Zero;

	m_canApplyRotationAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);
}

void FWK::RotationComponentModeBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::RotationComponentModeBase::PostDeserialize(const GameObject& a_owner)
{
	m_fetchComponentEventObserverFromSelfGameObjectHelper.PostDeserialize(a_owner);
	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize    (a_owner);
}

void FWK::RotationComponentModeBase::EditInspector()
{
	m_inspector.EditInspector(*this);
}

void FWK::RotationComponentModeBase::ResetRotationDirection()
{
	m_rotationDirection = TypeAlias::Math::Vector3::Zero;
}

nlohmann::json FWK::RotationComponentModeBase::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::RotationComponentModeBase::AddCanApplyRotationAxis(const Enum::Axis a_canApplyRotationAxis)
{
	// 同じ要素を含めない
	if (std::ranges::any_of(m_canApplyRotationAxisList, 
		                   [a_canApplyRotationAxis](const auto a_containsApplyRotationAxis) 
 	 	                   {
								return a_containsApplyRotationAxis == a_canApplyRotationAxis;
		                   }))
	{
		return;
	}

	// jsonに保存してもビットシフトの値が変わっても問題ないように
	// Enumをstd::vectorで保存する
	m_canApplyRotationAxisList.emplace_back(a_canApplyRotationAxis);

	// ビットフラグを反映する
	m_canApplyRotationAxis = Utility::EnableFlag(a_canApplyRotationAxis, m_canApplyRotationAxis);
}

bool FWK::RotationComponentModeBase::CanUpdate()
{
	const auto& l_fetchTransformComponent     = m_fetchTransformComponentFromSelfGameObjectHelper.GetREFFetchedTransformComponent        ().lock();
	const auto& l_fetchComponentEventObserver = m_fetchComponentEventObserverFromSelfGameObjectHelper.GetREFFetchedComponentEventObserver().lock();

	if (!l_fetchComponentEventObserver ||
		!l_fetchTransformComponent) 
	{
		return false; 
	}

	// イベントから回転できるかどうかを取得する
	// できないなら書いて処理を行わない
	if (!l_fetchComponentEventObserver->IsEventMatching(Enum::ComponentEvent::CanRotation, Enum::EventLane::TriggeredKeepFrame)) { return false; }
	
	return true;
}
