#include "GameObjectTransformComponent.h"

void FWK::GameObjectTransformComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectTransformComponent::PostDeserialize()
{
    if (!m_matrixUpdateMode) { return; }

    m_matrixUpdateMode->PostDeserialize(*this);

    UpdateMatrix();
}

void FWK::GameObjectTransformComponent::PostLateUpdate()
{
    // 更新する必要がなければ行列更新は重いのでスキップ
    if (!m_shouldUpdateMatrixDirty) { return; }

    UpdateMatrix();

    m_shouldUpdateMatrixDirty = false;

    // 自分のワールド行列が変わったので、子の行列も作り直す必要がある
    // 子は実行レベルが自分より深いため、同じフレームのPostLateUpdateで
    // 自分より後に実行され、更新済みの親行列を使って再計算される
    MarkChildrenMatrixDirty();
}

void FWK::GameObjectTransformComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::GameObjectTransformComponent::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::shared_ptr<FWK::GameObjectComponentBase> FWK::GameObjectTransformComponent::Clone() const
{
    auto l_clone = std::make_shared<GameObjectTransformComponent>();

    // INIT()はUUIDなどをリセットするため、値をコピーするDeserialize()より前に呼ぶ
    l_clone->INIT();

    // jsonに乗る部分(UUIDも含む)はDeserialize/Serializeで一括生成
    l_clone->Deserialize(Serialize());

    return l_clone;
}

void FWK::GameObjectTransformComponent::ApplyParent()
{
    const auto& l_ownerGameObject = GetREFOwner().lock();
 
    if (!l_ownerGameObject) { return; }
 
    const auto& l_ownerGameObjectHierarchy = l_ownerGameObject->GetREFHierarchy     ();
    const auto& l_parentGameObject         = l_ownerGameObjectHierarchy.GetREFParent().lock();
 
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
 
    // 親から自分まで行列を確定する
    // 親の行列が古いままだと逆行列計算が狂うため
    ConfirmMatrixFromRootToSelf();
 
    // 切替前のワールド行列を保存
    auto l_previousWorldMatrix = m_matrix;
 
    // 親のワールド行列を取得(ConfirmMatrixFromRootToSelfで確定済み)
    const auto& l_parentWorldMatrix = l_parentTransform->GetREFMatrix();
 
    // HierarchicalModeを生成
    auto l_matrixUpdateHierarchical = std::make_unique<GameObjectTransformComponentMatrixUpdateHierarchicalMode>();
 
    l_matrixUpdateHierarchical->EnableAllApplyCalculateParentWorldMatrixFlag();
 
    m_matrixUpdateMode = std::move(l_matrixUpdateHierarchical);
 
    // ポインタをつなげる処理などを行うためPostDeserializeする
    m_matrixUpdateMode->PostDeserialize(*this);
 
    // ワールド行列を保つようにローカルSRTを調整する
    m_matrixUpdateMode->PreserveWorldMatrix(l_parentWorldMatrix, *this, l_previousWorldMatrix);
 
    // 再計算 → 前のワールド行列が再現される
    UpdateMatrix();
}
void FWK::GameObjectTransformComponent::ApplyStandalone()
{
    // 親から自分まで行列を確定する
    ConfirmMatrixFromRootToSelf();
 
    // 切替前のワールド行列を保存
    auto l_previousWorldMatrix = m_matrix;
 
    // StandaloneModeを生成
    m_matrixUpdateMode = std::make_unique<GameObjectTransformComponentMatrixUpdateStandaloneMode>();
 
    // Standaloneでは親がいないのでParentWorld = Identity
    m_matrixUpdateMode->PreserveWorldMatrix(TypeAlias::Math::Matrix::Identity, *this, l_previousWorldMatrix);
 
    // 再計算 → 前のワールド行列が再現される
    UpdateMatrix();
}

void FWK::GameObjectTransformComponent::ApplyTransformScale(const TypeAlias::Math::Vector3& a_scale)
{
    m_transform.m_scale = a_scale;

    m_shouldUpdateMatrixDirty = true;
}
void FWK::GameObjectTransformComponent::ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_rotation)
{
    m_transform.m_rotation = a_rotation;

    m_shouldUpdateMatrixDirty = true;
}
void FWK::GameObjectTransformComponent::ApplyTransformPosition(const TypeAlias::Math::Vector3& a_position)
{
    m_transform.m_position = a_position;

    m_shouldUpdateMatrixDirty = true;
}

void FWK::GameObjectTransformComponent::UpdateMatrix()
{
    if (!m_matrixUpdateMode) { return; }

    m_matrixUpdateMode->UpdateMatrix(*this);
}

void FWK::GameObjectTransformComponent::MarkChildrenMatrixDirty() const
{
    const auto& l_ownerGameObject = GetREFOwner().lock();

    if (!l_ownerGameObject) { return; }

    const auto& l_ownerGameObjectHierarchy = l_ownerGameObject->GetREFHierarchy();
    const auto& l_childSmartPointerList    = l_ownerGameObjectHierarchy.GetREFChildSmartPointerVectorList();
    const auto& l_childDataList            = l_childSmartPointerList.GetREFElementDataList();

    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_childGameObject = l_childData.m_type.lock();

        if (!l_childGameObject) { continue; }

        const auto& l_childTransform = l_childGameObject->GetVALTransformComponent().lock();

        if (!l_childTransform) { continue; }

        // 子のdirtyを立てるだけで、行列計算は子自身のPostLateUpdateに任せる
        // 孫以降は、子のPostLateUpdateが同じ処理を行うことで連鎖的に伝わる
        l_childTransform->SetShouldUpdateMatrixDirty(true);
    }
}

void FWK::GameObjectTransformComponent::ConfirmMatrixFromRootToSelf() const
{
    // 自分からルートまでのTransformチェーンを収集
    std::vector<std::shared_ptr<GameObjectTransformComponent>> l_chain = {};
 
    auto l_currentGameObject = GetREFOwner().lock();
 
    while (l_currentGameObject)
    {
        const auto& l_transform = l_currentGameObject->GetVALTransformComponent().lock();
 
        if (!l_transform) { break; }
 
        l_chain.emplace_back(l_transform);

        const auto& l_currentGameObjectHierarchy = l_currentGameObject->GetREFHierarchy     ();
        const auto& l_parentGameObject           = l_currentGameObjectHierarchy.GetREFParent().lock();
 
        l_currentGameObject = l_parentGameObject ? l_parentGameObject : nullptr;
    }
 
    // ルート側から順にUpdateMatrix
    // 親が先に確定されていないと子の行列が正しくないため
    for (auto l_itr = l_chain.rbegin(); l_itr != l_chain.rend(); ++l_itr)
    {
        const auto& l_gameObjectTransformComponent = (*l_itr);

        l_gameObjectTransformComponent->UpdateMatrix();
    }
}