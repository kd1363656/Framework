#include "TransformComponent.h"

FWK::TransformComponent::TransformComponent() :
	m_parentTransformComponent({}),

	m_matrixStrategy(nullptr),

	m_transform()
{}
FWK::TransformComponent::~TransformComponent() = default;

void FWK::TransformComponent::ConfrimMatrix()
{
	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::ApplyParentTransformComponent(const std::weak_ptr<TransformComponent>& a_parentTransformComponent)
{
	m_parentTransformComponent = a_parentTransformComponent;

	// セットした後にダーティーフラグで行列の更新が妨げられてもいいように
	// ここで一度だけ行列を更新しておく
	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::ConfrimMatrixStrategy()
{
	if (!m_matrixStrategy) { return; }

	m_matrixStrategy->Execute(*this);
}