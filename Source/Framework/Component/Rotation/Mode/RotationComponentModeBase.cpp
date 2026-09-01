#include "RotationComponentModeBase.h"

void FWK::RotationComponentModeBase::INIT()
{
	m_rotationApplyAxisList.clear();

	m_fetchTransformComponentFromSelfGameObjectHelper = {};

	m_rotationDirection = TypeAlias::Math::Vector3::Zero;

	m_applyRotationAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);
}

void FWK::RotationComponentModeBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::RotationComponentModeBase::PostDeserialize(const std::shared_ptr<GameObject>& a_owner)
{
	if (!a_owner) { return; }

	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize(a_owner);
}

void FWK::RotationComponentModeBase::EditInspector()
{
	m_inspector.EditInspector(*this);
}

void FWK::RotationComponentModeBase::ResetRotationDirection()
{
	m_rotationDirection = TypeAlias::Math::Vector3::Zero;
}

nlohmann::json FWK::RotationComponentModeBase::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::RotationComponentModeBase::AddRotationApplyAxis(const Enum::Axis a_applyRotationAxis)
{
	// 同じ要素を含めない
	if (std::ranges::any_of(m_rotationApplyAxisList, [a_applyRotationAxis](const auto a_containsApplyRotationAxis) 
		                                                                  {
																				return a_containsApplyRotationAxis == a_applyRotationAxis;
		                                                                  }))
	{
		return;
	}

	// jsonに保存してもビットシフトの値が変わっても問題ないように
	// Enumをstd::vectorで保存する
	m_rotationApplyAxisList.emplace_back(a_applyRotationAxis);

	// ビットフラグを反映する
	Utility::EnableFlag(a_applyRotationAxis, m_applyRotationAxis);
}