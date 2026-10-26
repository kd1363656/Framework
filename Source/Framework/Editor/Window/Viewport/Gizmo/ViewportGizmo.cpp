#include "ViewportGizmo.h"

void FWK::Editor::ViewportGizmo::Draw(const EditorGameObjectSelectionState& a_gameObjectSelectionState,
                                      const Struct::CBCameraPass&           a_drawCameraConstantBuffer,
                                      const Struct::ViewportScreenRect&     a_viewportRect,
                                      const Enum::ViewportGizmoOperation    a_gizmoOperation)
{
    std::vector<std::shared_ptr<GameObject>> l_gameObjectList = {};

    // 掴んでいない間は、選択が変わっていてもすぐ追従できるよう毎フレームギズモの位置を決め直す
    if (!m_isManipulating)
    {
        CollectTargetGameObjectList(a_gameObjectSelectionState, l_gameObjectList);

        m_isVisible = !l_gameObjectList.empty();

        if (!m_isVisible) { return; }

        m_pivotMatrix = CalculatePivotMatrix(l_gameObjectList);
    }

    // ImGuizmoはビューポートのウィンドウの描画リストへ、ビューポートの範囲に合わせて描く
    ImGuizmo::SetDrawlist    (ImGui::GetWindowDrawList());
    ImGuizmo::SetOrthographic(k_isOrthographic);

    // ImGuizmo::SetRect(描く範囲の左上x、左上y、幅、高さ)
    ImGuizmo::SetRect(a_viewportRect.m_min.x,
                      a_viewportRect.m_min.y,
                      a_viewportRect.m_size.x,
                      a_viewportRect.m_size.y);

    // 掴んだ瞬間の行列を覚えるため、ImGuizmoが書き換える前の行列を取っておく
    const auto l_pivotMatrixBeforeManipulate = m_pivotMatrix;

    // ImGuizmo::Manipulate(ビュー行列、射影行列、操作の種類、ワールド軸かローカル軸か、動かす行列)
    // 行列はfloat16個の並びとして渡す(DirectXの行列の並びのまま渡せる)
    // 掴んで動かしたフレームはtrueを返し、動かす行列が書き換わる
    // ※ 拡大はImGuizmoの決まりでローカル軸になるが、ギズモの行列は回転が無いのでワールド軸と同じになる
    ImGuizmo::Manipulate(&a_drawCameraConstantBuffer.m_viewMatrix.m[0][0],
                         &a_drawCameraConstantBuffer.m_projectionMatrix.m[0][0],
                         ConvertImGuizmoOperation(a_gizmoOperation),
                         ImGuizmo::WORLD,
                         &m_pivotMatrix.m[0][0]);

    const bool l_isUsing = ImGuizmo::IsUsing();

    // 掴んだ瞬間 : 各GameObjectの今の状態を覚える
    if (l_isUsing &&
        !m_isManipulating)
    {
        BeginManipulate(l_gameObjectList, l_pivotMatrixBeforeManipulate);
    }

    // 掴んでいる間 : ギズモの動きを全員に掛ける
    if (l_isUsing)
    {
        UpdateManipulate();

        return;
    }

    // 離した瞬間 : 動かす前と後をUndo / Redoへ積む
    if (m_isManipulating)
    {
        EndManipulate();
    }
}

bool FWK::Editor::ViewportGizmo::FetchVALIsOverOrUsing() const
{
    if (!m_isVisible) { return false; }

    return ImGuizmo::IsOver () ||
           ImGuizmo::IsUsing();
}

void FWK::Editor::ViewportGizmo::CollectTargetGameObjectList(const EditorGameObjectSelectionState& a_gameObjectSelectionState, std::vector<std::shared_ptr<GameObject>>& a_outGameObjectList) const
{
    const auto& l_selectedGameObjectList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();

    a_outGameObjectList.reserve(l_selectedGameObjectList.size());

    for (const auto& l_gameObjectWeak : l_selectedGameObjectList)
    {
        const auto& l_gameObject = l_gameObjectWeak.lock();

        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        // 親(祖先)も選択されているGameObjectは、親を動かすだけで一緒に動くため対象にしない
        // (対象にすると、親の分と自分の分で2回動いてしまう)
        if (IsSelectedAncestor(*l_gameObject, a_gameObjectSelectionState)) { continue; }

        a_outGameObjectList.emplace_back(l_gameObject);
    }
}

FWK::TypeAlias::Math::Matrix FWK::Editor::ViewportGizmo::CalculatePivotMatrix(const std::vector<std::shared_ptr<GameObject>>& a_gameObjectList) const
{
    // 全員の位置のXYZそれぞれの最小値と最大値を求め、その真ん中((最小 + 最大) ÷ 2)にギズモを置く
    auto l_minPosition = TypeAlias::Math::Vector3{  FLT_MAX,  FLT_MAX,  FLT_MAX };
    auto l_maxPosition = TypeAlias::Math::Vector3{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (const auto& l_gameObject : a_gameObjectList)
    {
        const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

        if (!l_transformComponent) { continue; }

        const auto& l_worldMatrix = l_transformComponent->GetREFMatrix();
        const auto& l_position    = l_worldMatrix.Translation      ();

        l_minPosition = TypeAlias::Math::Vector3::Min(l_minPosition, l_position);
        l_maxPosition = TypeAlias::Math::Vector3::Max(l_maxPosition, l_position);
    }

    const auto& l_centerPosition = (l_minPosition + l_maxPosition) * Constant::k_halfMagnification;

    // 回転、拡大の無い行列にしておく(ワールド軸で回し、拡大の基準も分かりやすくするため)
    return TypeAlias::Math::Matrix::CreateTranslation(l_centerPosition);
}

void FWK::Editor::ViewportGizmo::BeginManipulate(const std::vector<std::shared_ptr<GameObject>>& a_gameObjectList, const TypeAlias::Math::Matrix& a_beginPivotMatrix)
{
    m_manipulateTargetList.clear  ();
    m_manipulateTargetList.reserve(a_gameObjectList.size());

    for (const auto& l_gameObject : a_gameObjectList)
    {
        const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

        if (!l_transformComponent) { continue; }

        ManipulateTarget l_target = {};

        l_target.m_gameObject       = l_gameObject;
        l_target.m_beginWorldMatrix = l_transformComponent->GetREFMatrix   ();
        l_target.m_beginTransform   = l_transformComponent->GetREFTransform();

        m_manipulateTargetList.emplace_back(l_target);
    }

    m_beginPivotMatrix = a_beginPivotMatrix;
    m_isManipulating   = true;
}
void FWK::Editor::ViewportGizmo::UpdateManipulate()
{
    // 掴んだ瞬間のギズモから見た位置関係を、今のギズモへそのまま持っていく行列
    // (DirectXは「行列を右へ掛けていく」順番なので、ワールド行列 × P0の逆行列 × P1 の順に掛ける)
    const auto& l_deltaMatrix = m_beginPivotMatrix.Invert() * m_pivotMatrix;

    for (const auto& l_target : m_manipulateTargetList)
    {
        const auto& l_gameObject = l_target.m_gameObject.lock();

        if (!l_gameObject) { continue; }

        // 掴んだ瞬間の行列から毎フレーム計算し直す(前のフレームの結果へ足していくと、少しずつ誤差がたまるため)
        ApplyWorldMatrix(*l_gameObject, l_target.m_beginWorldMatrix * l_deltaMatrix);
    }
}
void FWK::Editor::ViewportGizmo::EndManipulate()
{
    std::vector<Struct::TransformGameObjectRecord> l_recordList = {};

    l_recordList.reserve(m_manipulateTargetList.size());

    for (const auto& l_target : m_manipulateTargetList)
    {
        const auto& l_gameObject = l_target.m_gameObject.lock();

        if (!l_gameObject) { continue; }

        const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

        if (!l_transformComponent) { continue; }

        const auto& l_afterTransform = l_transformComponent->GetREFTransform();

        // 動いていない物は記録しない
        if (IsSameTransform(l_target.m_beginTransform, l_afterTransform)) { continue; }

        Struct::TransformGameObjectRecord l_record = {};

        l_record.m_beforeTransform   = l_target.m_beginTransform;
        l_record.m_afterTransform    = l_afterTransform;
        l_record.m_sceneInstanceUUID = l_gameObject->GetREFSceneInstanceUUID();

        l_recordList.emplace_back(l_record);
    }

    m_manipulateTargetList.clear();

    m_isManipulating = false;

    if (l_recordList.empty()) { return; }

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<TransformGameObjectCommand>(std::move(l_recordList));
}

void FWK::Editor::ViewportGizmo::ApplyWorldMatrix(GameObject& a_gameObject, const TypeAlias::Math::Matrix& a_worldMatrix) const
{
    const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

    if (!l_transformComponent) { return; }

    auto& l_matrixUpdateMode = l_transformComponent->GetMutableREFMatrixUpdateMode();

    if (!l_matrixUpdateMode) { return; }

    // 親の行列を使うモード(Hierarchical)なら親のワールド行列、使わないモード(Standalone)なら単位行列を親として扱う
    auto l_parentWorldMatrix = TypeAlias::Math::Matrix::Identity;

    const auto& l_modeTypeINFO         = l_matrixUpdateMode->GetREFRuntimeTypeINFO                             ();
    const auto& l_hierarchicalTypeINFO = GameObjectTransformComponentMatrixUpdateHierarchicalMode::GetREFTypeINFO();

    if (l_modeTypeINFO.k_staticTypeID == l_hierarchicalTypeINFO.k_staticTypeID)
    {
        const auto& l_hierarchy = a_gameObject.GetREFHierarchy();
        const auto& l_parent    = l_hierarchy.GetREFParent    ().lock();

        if (l_parent)
        {
            if (const auto& l_parentTransformComponent = l_parent->GetVALTransformComponent().lock();
                l_parentTransformComponent)
            {
                l_parentWorldMatrix = l_parentTransformComponent->GetREFMatrix();
            }
        }
    }

    // ワールド行列から、親に対するローカルの拡大、回転、位置へ分解して書き込む(親子付けの切り替えと同じ処理)
    auto l_worldMatrix = a_worldMatrix;

    l_matrixUpdateMode->PreserveWorldMatrix(l_parentWorldMatrix, *l_transformComponent, l_worldMatrix);

    // 行列はPostLateUpdateで作り直される(子の行列も合わせて作り直される)
    l_transformComponent->SetShouldUpdateMatrixDirty(true);
}

bool FWK::Editor::ViewportGizmo::IsSelectedAncestor(const GameObject& a_gameObject, const EditorGameObjectSelectionState& a_gameObjectSelectionState) const
{
    const auto& l_hierarchy = a_gameObject.GetREFHierarchy();
          auto  l_parent    = l_hierarchy.GetREFParent    ().lock();

    while (l_parent)
    {
        if (a_gameObjectSelectionState.FindVALIsSelected(l_parent)) { return true; }

        const auto& l_parentHierarchy = l_parent->GetREFHierarchy();

        l_parent = l_parentHierarchy.GetREFParent().lock();
    }

    return false;
}
bool FWK::Editor::ViewportGizmo::IsSameTransform(const Struct::Transform& a_transformA, const Struct::Transform& a_transformB) const
{
    return a_transformA.m_scale    == a_transformB.m_scale    &&
           a_transformA.m_rotation == a_transformB.m_rotation &&
           a_transformA.m_position == a_transformB.m_position;
}

ImGuizmo::OPERATION FWK::Editor::ViewportGizmo::ConvertImGuizmoOperation(const Enum::ViewportGizmoOperation a_gizmoOperation)
{
    switch (a_gizmoOperation)
    {
        case Enum::ViewportGizmoOperation::Rotate: return ImGuizmo::ROTATE;
        case Enum::ViewportGizmoOperation::Scale:  return ImGuizmo::SCALE;
        default:                                   return ImGuizmo::TRANSLATE;
    }
}
