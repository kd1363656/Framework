#include "RotationComponentModeBase.h"

void FWK::RotationComponentModeBase::INIT()
{
	m_rotationApplyAxisList.clear();

	m_fetchTransformComponentFromSelfGameObjectHelper = {};

	m_rotationDirection = TypeAlias::Math::Vector3::Zero;

	m_applyRotationAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);
}

void FWK::RotationComponentModeBase::PostDeserialize(const std::shared_ptr<GameObject>& a_owner)
{
	if (!a_owner) { return; }

	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize(a_owner);
}

void FWK::RotationComponentModeBase::ResetRotationDirection()
{
	m_rotationDirection = TypeAlias::Math::Vector3::Zero;
}