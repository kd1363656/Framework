#include "GameObjectTransformComponentMatrixUpdateStandaloneMode.h"

void FWK::GameObjectTransformComponentMatrixUpdateStandaloneMode::UpdateMatrix(GameObjectTransformComponent& a_transformComponent)
{
    auto&& l_resultMatrix = a_transformComponent.CalculateScaleMatrix() * a_transformComponent.CalculateRotationMatrix() * a_transformComponent.CalculateTranslationMatrix();

    a_transformComponent.SetMatrix(std::move(l_resultMatrix));
}

void FWK::GameObjectTransformComponentMatrixUpdateStandaloneMode::PreserveWorldMatrix(const TypeAlias::Math::Matrix&, GameObjectTransformComponent& a_transformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix)
{
    // StandaloneではLocal = World なので
    // 前のワールド行列をそのままSRTへ分解する
    auto& l_transform = a_transformComponent.GetMutableREFTransform();

    a_previousWorldMatrix.Decompose(l_transform.m_scale, l_transform.m_rotation, l_transform.m_position);
}