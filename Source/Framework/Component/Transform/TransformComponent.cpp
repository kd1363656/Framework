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

FWK::TypeAlias::Math::Matrix FWK::TransformComponent::CalculateWorldMatrixData(const Utility::EnumBitShift<Enum::ApplyCalculateWorldMatrixBitShiftFlag>& a_calculateParentWorldMatrixEnumBitShift) const
{
    const auto& l_parentGameObject = m_parentGameObject.lock();

    if (!l_parentGameObject) { return TypeAlias::Math::Matrix::Identity; }

    const auto& l_parent = l_parentGameObject->GetVALTransformComponent().lock();

    if (!l_parent) {return TypeAlias::Math::Matrix::Identity; }

    // 選択成分をS * R * Tの順番で合成
    // この順序はApplyParentの逆行列計算と一致させるため必ずこの関数経由にする
    auto l_resultMatrix = TypeAlias::Math::Matrix::Identity;

    if (a_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale)    ||
        a_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation) ||
        a_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position))
    {
        auto& l_parentMatrix = l_parent->GetMutableREFMatrix();

        TypeAlias::Math::Vector3    l_scale    = {};
        TypeAlias::Math::Quaternion l_rotation = {};
        TypeAlias::Math::Vector3    l_position = {};

        l_parentMatrix.Decompose(l_scale, l_rotation, l_position);

        if (a_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale))
        {
            l_resultMatrix *= TypeAlias::Math::Matrix::CreateScale(l_scale);
        }
        if (a_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation))
        {
            l_resultMatrix *= TypeAlias::Math::Matrix::CreateFromQuaternion(l_rotation);
        }
        if (a_calculateParentWorldMatrixEnumBitShift.IsFlagEnabled(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position))
        {
            l_resultMatrix *= TypeAlias::Math::Matrix::CreateTranslation(l_position);
        }
    }

    return l_resultMatrix;
}

void FWK::TransformComponent::ApplyParent(const std::weak_ptr<GameObject>& a_parentGameObject)
{
    const auto& l_parentGameObject = a_parentGameObject.lock();
 
    if (!l_parentGameObject)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親GameObjectが無効のため、ApplyParentに失敗しました。");
 
        return;
    }
 
    const auto& l_parentTransform = l_parentGameObject->GetVALTransformComponent().lock();
 
    if (!l_parentTransform)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親のTransformComponentが無効のため、ApplyParentに失敗しました。");
 
        return;
    }
 
    // 現在のワールド行列を確定させてから計算する
    // ConfirmMatrixはprivateだが同一クラスなので親側インスタンスからも呼べる
    ConfirmMatrix();
 
    // 親の行列が古いままだと逆行列計算が狂うので先に確定させる
    l_parentTransform->ConfirmMatrix();
 
    // World = Local * ParentWorld より Local = World * ParentWorldの逆行列
    auto l_localMatrix = m_matrix * l_parentTransform->GetREFMatrix().Invert();
 
    // ローカル行列をSRTへ分解して保持する
    l_localMatrix.Decompose(m_transform.m_scale, m_transform.m_rotation, m_transform.m_position);
 
    m_parentGameObject = a_parentGameObject;
 
    // Factory経由で生成(InspectorやDeserializeと同じ経路)
    m_matrixStrategy = std::make_unique<HierarchicalMatrixStrategy>();
 
    if (!m_matrixStrategy)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "HierarchicalMatrixStrategyの生成に失敗しました。");
 
        return;
    }
 
    ConfirmMatrix();
}
void FWK::TransformComponent::ApplyStandalone()
{
    // 現在のワールド行列を確定させてからローカルへ落とす
    ConfirmMatrix();

    // Standaloneではローカル=ワールドなので
    // 現在のワールド行列をそのままSRTへ分解して保持する
    m_matrix.Decompose(m_transform.m_scale, m_transform.m_rotation, m_transform.m_position);

    // 親参照を切る(Hierarchicalでも参照が残って誤動作しないように)
    m_parentGameObject.reset();

    m_matrixStrategy = std::make_unique<StandaloneMatrixStrategy>();

    if (!m_matrixStrategy)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "StandaloneMatrixStrategyの生成に失敗しました。");

        return;
    }

    ConfirmMatrix();
}

void FWK::TransformComponent::EnableAllApplyCalculateWorldMatrixFlag()
{
    // 親行列を使わないStrategyなら使う必要のない変数なのでreturn
    if (!m_matrixStrategy)                      { return; }
    if (!m_matrixStrategy->IsUseParentMatrix()) { return; }

    if (!m_calculateParentWorldMatrixEnumBitShift)
    {
        m_calculateParentWorldMatrixEnumBitShift = std::make_unique<Utility::EnumBitShift<Enum::ApplyCalculateWorldMatrixBitShiftFlag>>();
    }

    auto& l_enumBitShift = *m_calculateParentWorldMatrixEnumBitShift;

    // 全成分(Scale|Rotation|Position)を適用
    l_enumBitShift.AddBitShiftFlag(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale);
    l_enumBitShift.AddBitShiftFlag(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation);
    l_enumBitShift.AddBitShiftFlag(Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position);
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