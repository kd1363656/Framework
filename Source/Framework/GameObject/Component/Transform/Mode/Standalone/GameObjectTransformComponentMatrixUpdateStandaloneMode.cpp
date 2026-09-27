#include "GameObjectTransformComponentMatrixUpdateStandaloneMode.h"

void FWK::GameObjectTransformComponentMatrixUpdateStandaloneMode::UpdateMatrix(GameObjectTransformComponent& a_gameObjectTransformComponent)
{
    auto l_resultMatrix = a_gameObjectTransformComponent.CalculateScaleMatrix() * a_gameObjectTransformComponent.CalculateRotationMatrix() * a_gameObjectTransformComponent.CalculateTranslationMatrix();

    a_gameObjectTransformComponent.SetMatrix(std::move(l_resultMatrix));
}

void FWK::GameObjectTransformComponentMatrixUpdateStandaloneMode::PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_gameObjectTransformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix)
{
    // StandaloneではLocal = World なので
    // 前のワールド行列をそのままSRTへ分解する
    auto& l_transform = a_gameObjectTransformComponent.GetMutableREFTransform();

    a_previousWorldMatrix.Decompose(l_transform.m_scale, l_transform.m_rotation, l_transform.m_position);
}