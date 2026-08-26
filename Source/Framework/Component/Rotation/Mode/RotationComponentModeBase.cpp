#include "RotationComponentModeBase.h"

void FWK::RotationComponentModeBase::PostDeserialize(const std::shared_ptr<GameObject>& a_owner)
{
	if (!a_owner) { return; }

	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize(a_owner);
}