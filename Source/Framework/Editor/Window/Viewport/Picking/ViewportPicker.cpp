#include "ViewportPicker.h"

void FWK::Editor::ViewportPicker::Draw(const Scene&                          a_scene,
                                       const EditorGameObjectSelectionState& a_gameObjectSelectionState,
                                       const Struct::CBCameraPass&           a_drawCameraConstantBuffer,
                                       const Struct::ViewportScreenRect&     a_viewportRect,
                                       const bool                            a_isViewportHovered)
{
    m_hoveredGameObject = {};

    auto* const l_drawList = ImGui::GetWindowDrawList();

    const auto& l_mousePosition = ImGui::GetMousePos();

    // マウスに一番近い目印のGameObjectを「マウスの下にある物」とする
    float l_nearestDistanceSquared = k_pickRadius * k_pickRadius;

    for (const auto& l_gameObject : a_scene.GetREFGameObjectList())
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

        if (!l_transformComponent) { continue; }

        const auto& l_componentContainer = l_gameObject->GetREFComponentContainer                              ();
        const auto& l_cameraComponent    = l_componentContainer.FindUniqueComponent<GameObjectCameraComponent>().lock();

        // 今ビューポートを描いているカメラそのものは、画面の真ん中(カメラの位置)に重なって邪魔になるため描かない
        if (l_cameraComponent)
        {
            const auto& l_camera = l_cameraComponent->GetREFCamera();

            if (l_camera.GetREFCBCameraPass().get() == &a_drawCameraConstantBuffer) { continue; }
        }

        const auto& l_worldMatrix   = l_transformComponent->GetREFMatrix();
        const auto& l_worldPosition = l_worldMatrix.Translation        ();

        // GameObjectのワールド座標を、ビューポートの画面上の座標へ変換する(カメラの後ろなら描かない)
        ImVec2 l_screenPosition = {};

        if (!Utility::ProjectViewportWorldToScreen(l_worldPosition,
                                                   a_drawCameraConstantBuffer.m_viewProjectionMatrix,
                                                   a_viewportRect,
                                                   l_screenPosition))
        {
            continue;
        }

        const float l_distanceX       = l_screenPosition.x - l_mousePosition.x;
        const float l_distanceY       = l_screenPosition.y - l_mousePosition.y;
        const float l_distanceSquared = l_distanceX * l_distanceX + l_distanceY * l_distanceY;

        const bool l_isHovered = a_isViewportHovered &&
                                 l_distanceSquared <= l_nearestDistanceSquared;

        if (l_isHovered)
        {
            l_nearestDistanceSquared = l_distanceSquared;
            m_hoveredGameObject      = l_gameObject;
        }

        const auto l_color = FetchVALIconColor(a_gameObjectSelectionState.FindVALIsSelected(l_gameObject), l_isHovered);

        if (l_cameraComponent)
        {
            DrawCameraIcon(*l_drawList, l_screenPosition, l_color);

            continue;
        }

        DrawMarker(*l_drawList, l_screenPosition, l_color);
    }
}

void FWK::Editor::ViewportPicker::SelectHoveredGameObject(EditorManager& a_editorManager, const bool a_isToggleSelection) const
{
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();

    // SceneとGameObjectは同時に選択しない決まり(WorldOutlinerと同じ)なので、Sceneの選択状態も合わせて扱う
    const auto& l_outlinerEditorWindow = a_editorManager.FindVALWindowEditor<WorldOutlinerEditorWindow>().lock();

    bool l_beforeIsSceneSelected = false;

    if (l_outlinerEditorWindow)
    {
        const auto& l_sceneSelectionState = l_outlinerEditorWindow->GetREFSceneSelectionState();

        l_beforeIsSceneSelected = l_sceneSelectionState.GetVALIsSceneSelected();
    }

    // Undo / Redo用に、変更前の選択状態を写しておく
    std::vector<boost::uuids::uuid> l_beforeUUIDList   = {};
    boost::uuids::uuid              l_beforeAnchorUUID = {};

    Utility::FetchEditorGameObjectSelectionSnapshot(l_gameObjectSelectionState, l_beforeUUIDList, l_beforeAnchorUUID);

    if (!m_hoveredGameObject.expired())
    {
        // Ctrl / Shift + クリック : 追加 / 解除
        // 通常クリック          : そのGameObjectだけを選択
        if (a_isToggleSelection) { l_gameObjectSelectionState.ToggleSelectedGameObject(m_hoveredGameObject); }
        else                     { l_gameObjectSelectionState.SelectSingleGameObject  (m_hoveredGameObject); }
    }
    else if (!a_isToggleSelection)
    {
        // 何も無い所の通常クリックは選択の解除
        l_gameObjectSelectionState.ClearSelectedGameObjectList();
    }

    if (l_outlinerEditorWindow)
    {
        auto& l_sceneSelectionState = l_outlinerEditorWindow->GetMutableREFSceneSelectionState();

        l_sceneSelectionState.SetIsSceneSelected(false);
    }

    Utility::PushEditorGameObjectSelectionChangeCommand(l_beforeIsSceneSelected,
                                                        k_afterIsSceneSelected,
                                                        std::move(l_beforeUUIDList),
                                                        std::move(l_beforeAnchorUUID),
                                                        l_gameObjectSelectionState);
}

void FWK::Editor::ViewportPicker::DrawCameraIcon(ImDrawList& a_drawList, const ImVec2& a_screenPosition, const ImU32 a_color) const
{
    // カメラのアイコン(FontAwesomeの文字)を、位置が中心になるように描く
    auto* const l_font     = ImGui::GetFont();
    const auto& l_textSize = l_font->CalcTextSizeA(k_cameraIconFontSize,
                                                   k_unlimitedTextWidth,
                                                   k_noTextWrapWidth,
                                                   k_cameraIconText.data());

    const ImVec2 l_textPosition = { a_screenPosition.x - l_textSize.x * k_halfSize,
                                    a_screenPosition.y - l_textSize.y * k_halfSize };

    const ImVec2 l_shadowPosition = { l_textPosition.x + k_iconShadowOffset.x,
                                      l_textPosition.y + k_iconShadowOffset.y };

    // 明るい背景でも見えるように、少しずらした影を先に描く
    // AddText(フォント、文字の大きさ、位置、色、文字列)
    a_drawList.AddText(l_font,
                       k_cameraIconFontSize,
                       l_shadowPosition,
                       k_iconShadowColor,
                       k_cameraIconText.data());

    a_drawList.AddText(l_font,
                       k_cameraIconFontSize,
                       l_textPosition,
                       a_color,
                       k_cameraIconText.data());
}
void FWK::Editor::ViewportPicker::DrawMarker(ImDrawList& a_drawList, const ImVec2& a_screenPosition, const ImU32 a_color) const
{
    a_drawList.AddCircleFilled(a_screenPosition, k_markerRadius + k_markerOutline, k_iconShadowColor);
    a_drawList.AddCircleFilled(a_screenPosition, k_markerRadius,                   a_color);
}

ImU32 FWK::Editor::ViewportPicker::FetchVALIconColor(const bool a_isSelected, const bool a_isHovered) const
{
    if (a_isSelected) { return k_iconSelectedColor; }
    if (a_isHovered)  { return k_iconHoveredColor; }

    return k_iconColor;
}
