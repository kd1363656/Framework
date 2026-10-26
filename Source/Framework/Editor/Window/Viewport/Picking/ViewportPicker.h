#pragma once

namespace FWK
{
    class GameObject;
    class Scene;
}

namespace FWK::Editor
{
    class EditorManager;
    class EditorGameObjectSelectionState;
}

namespace FWK::Editor
{
    // ビューポート上にGameObjectの目印(カメラはカメラのアイコン、それ以外は小さな丸)を描き、
    // クリックした目印のGameObjectを選択するクラス
    //
    // 選択は通常クリックで1つだけ、Ctrl又はShiftを押しながらのクリックで追加 / 解除(複数選択)
    // 何も無い所の通常クリックで選択を解除する
    // 選択の変更はWorldOutlinerと同じくUndo / Redoへ積む
    class ViewportPicker final
    {
    public:

         ViewportPicker() = default;
        ~ViewportPicker() = default;

        void Draw(const Scene&                           a_scene,
                  const EditorGameObjectSelectionState&  a_gameObjectSelectionState,
                  const Struct::CBCameraPass&            a_drawCameraConstantBuffer,
                  const Struct::ViewportScreenRect&      a_viewportRect,
                  const bool                             a_isViewportHovered);

        void SelectHoveredGameObject(EditorManager& a_editorManager, const bool a_isToggleSelection) const;

    private:

        void DrawCameraIcon(ImDrawList& a_drawList, const ImVec2& a_screenPosition, const ImU32 a_color) const;
        void DrawMarker    (ImDrawList& a_drawList, const ImVec2& a_screenPosition, const ImU32 a_color) const;

        ImU32 FetchVALIconColor(const bool a_isSelected, const bool a_isHovered) const;

        static constexpr std::string_view k_cameraIconText = "\xEF\x80\xB0";

        static constexpr ImU32 k_iconColor         = IM_COL32(230, 230, 230, 255);
        static constexpr ImU32 k_iconHoveredColor  = IM_COL32(255, 255, 255, 255);
        static constexpr ImU32 k_iconSelectedColor = IM_COL32(0, 140, 255, 255);
        static constexpr ImU32 k_iconShadowColor   = IM_COL32(0, 0, 0, 200);

        static constexpr float k_cameraIconFontSize = 22.0F;
        static constexpr float k_markerRadius       = 4.0F;
        static constexpr float k_markerOutline      = 1.5F;
        static constexpr float k_pickRadius         = 14.0F;
        static constexpr float k_halfSize           = 0.5F;
        static constexpr float k_unlimitedTextWidth = FLT_MAX;
        static constexpr float k_noTextWrapWidth    = 0.0F;

        static constexpr ImVec2 k_iconShadowOffset = { 1.0F, 1.0F };

        static constexpr bool k_afterIsSceneSelected = false;

        std::weak_ptr<GameObject> m_hoveredGameObject = {};
    };
}
