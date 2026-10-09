#include "GameObjectTransformComponentMatrixUpdateHierarchicalMode.h"

void FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::PostDeserialize(const GameObjectTransformComponent& a_transformComponent)
{
    const auto& l_owner = a_transformComponent.GetREFOwner().lock();

    if (!l_owner) { return; }

    const auto& l_ownerHierarchy   = l_owner->GetREFHierarchy     ();
    const auto& l_parentGameObject = l_ownerHierarchy.GetREFParent().lock();

    if (!l_parentGameObject) { return; }

    const auto& l_parent = l_parentGameObject->GetVALTransformComponent();

    if (l_parent.expired()) { return; }

    m_parentTransformComponent = l_parent;
}

void FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::UpdateMatrix(GameObjectTransformComponent& a_transformComponent)
{
    // 親の行列が先に更新されていることを前提にしている処理
    auto l_resultMatrix = TypeAlias::Math::Matrix::Identity;

    // 回転してから移動するか、移動してから回転する違い
    // 移動してから回転する挙動は原点を中心に回転する
    if (GetVALIsRotateAroundPosition())
    {
        l_resultMatrix = a_transformComponent.CalculateScaleMatrix()       *
                         a_transformComponent.CalculateTranslationMatrix() *
                         a_transformComponent.CalculateRotationMatrix()    *
                         CalculateParentWorldMatrix();
    }
    else
    {
        l_resultMatrix = a_transformComponent.CalculateScaleMatrix()       *
                         a_transformComponent.CalculateRotationMatrix()    *
                         a_transformComponent.CalculateTranslationMatrix() *
                         CalculateParentWorldMatrix();
    }

    a_transformComponent.SetMatrix(std::move(l_resultMatrix));
}

void FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_transformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix)
{
    // HierarchicalではWorld = S * R * T * ParentWorld なので
    // Local = World * 親の逆行列
    auto  l_localMatrix = a_previousWorldMatrix * a_parentWorldMatrix.Invert();
    auto& l_transform   = a_transformComponent.GetMutableREFTransform       ();

    l_localMatrix.Decompose(l_transform.m_scale, l_transform.m_rotation, l_transform.m_position);
}

void FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::EnableAllApplyCalculateParentWorldMatrixFlag()
{
    // 全成分(Scale|Rotation|Position)を適用
    m_calculateParentWorldMatrixEnumBitShift.AddBitShiftFlag(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale);
    m_calculateParentWorldMatrixEnumBitShift.AddBitShiftFlag(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation);
    m_calculateParentWorldMatrixEnumBitShift.AddBitShiftFlag(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position);
}

FWK::TypeAlias::Math::Matrix FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode::CalculateParentWorldMatrix() const
{
    const auto& l_parent = m_parentTransformComponent.lock();

    if (!l_parent) {return TypeAlias::Math::Matrix::Identity; }

    // 選択成分をS * R * Tの順番で合成
    // この順序はApplyParentの逆行列計算と一致させるため必ずこの関数経由にする
    auto l_resultMatrix = TypeAlias::Math::Matrix::Identity;

    if (m_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale) ||
        m_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation) ||
        m_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position))
    {
        auto& l_parentMatrix = l_parent->GetMutableREFMatrix();

        TypeAlias::Math::Vector3    l_scale    = {};
        TypeAlias::Math::Quaternion l_rotation = {};
        TypeAlias::Math::Vector3    l_position = {};

        l_parentMatrix.Decompose(l_scale, l_rotation, l_position);

        if (m_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale))
        {
            l_resultMatrix *= TypeAlias::Math::Matrix::CreateScale(l_scale);
        }
        if (m_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation))
        {
            l_resultMatrix *= TypeAlias::Math::Matrix::CreateFromQuaternion(l_rotation);
        }
        if (m_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position))
        {
            l_resultMatrix *= TypeAlias::Math::Matrix::CreateTranslation(l_position);
        }
    }

    return l_resultMatrix;
}