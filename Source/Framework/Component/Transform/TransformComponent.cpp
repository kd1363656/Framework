#include "TransformComponent.h"

void FWK::TransformComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::TransformComponent::PostDeserialize()
{
    ConfirmMatrix();
}

void FWK::TransformComponent::PostLateUpdate()
{
    // 更新する必要がなければ行列更新は重いのでスキップ
    if (!m_shouldUpdateMatrixDirty) { return; }

    ConfirmMatrix();

    m_shouldUpdateMatrixDirty = false;
}

void FWK::TransformComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::TransformComponent::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::TransformComponent::ApplyTransformScale(const TypeAlias::Math::Vector3& a_scale)
{
    m_transform.m_scale = a_scale;

    m_shouldUpdateMatrixDirty = true;
}
void FWK::TransformComponent::ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_rotation)
{
    m_transform.m_rotation = a_rotation;

    m_shouldUpdateMatrixDirty = true;
}
void FWK::TransformComponent::ApplyTransformPosition(const TypeAlias::Math::Vector3& a_position)
{
    m_transform.m_position = a_position;

    m_shouldUpdateMatrixDirty = true;
}

void FWK::TransformComponent::ConfirmMatrix()
{
    if (!m_matrixStrategy) { return; }

    m_matrixStrategy->Execute(*this);
}