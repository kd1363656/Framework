#pragma once

namespace FWK::Editor
{
    class ViewportDebugCamera;
}

namespace FWK::Editor
{
    // ビューポートのTextureの上に重ねて描くボタン(▶と■は絵の上のツールバー、ViewportToolbarにある)
    //
    // 右上 : 移動、回転、拡大(ギズモの操作の切り替え。選んでいる物を青くする)と、その右にカメラと▽をまとめたボタン
    //        カメラのボタンを押すと、デバッグカメラの設定を開く(デバッグビルドのみ)
    class ViewportOverlayToolbar final
    {
    public:

         ViewportOverlayToolbar() = default;
        ~ViewportOverlayToolbar() = default;

        void Draw(const Struct::ViewportScreenRect& a_viewportRect, Enum::ViewportGizmoOperation& a_gizmoOperation, ViewportDebugCamera& a_debugCamera) const;

    private:

        void DrawGizmoOperationButtonGroup(const ImVec2& a_position, Enum::ViewportGizmoOperation& a_gizmoOperation) const;

        void DrawDebugCameraButton(const ImVec2& a_position, ViewportDebugCamera& a_debugCamera) const;

        static constexpr std::string_view k_translateIconLabel   = "\xEF\x82\xB2##ViewportGizmoTranslate";
        static constexpr std::string_view k_rotateIconLabel      = "\xEF\x8B\xB1##ViewportGizmoRotate";
        static constexpr std::string_view k_scaleIconLabel       = "\xEF\x81\xA5##ViewportGizmoScale";
        static constexpr std::string_view k_debugCameraIconLabel = "\xEF\x80\xB0 \xEF\x83\x97##ViewportDebugCamera";

        static constexpr std::string_view k_translateTooltip   = "移動";
        static constexpr std::string_view k_rotateTooltip      = "回転";
        static constexpr std::string_view k_scaleTooltip       = "拡大";
        static constexpr std::string_view k_debugCameraTooltip = "デバッグカメラの設定";

        static constexpr std::string_view k_debugCameraPopupID = "##ViewportDebugCameraSettingsPopup";

        static constexpr ImVec2 k_iconButtonSize        = { 28.0F, 28.0F };
        static constexpr ImVec2 k_debugCameraButtonSize = { 44.0F, 28.0F };

        static constexpr float k_overlayPadding = 8.0F;
        static constexpr float k_groupSpacing   = 12.0F;
        static constexpr float k_buttonSpacing  = 2.0F;

        static constexpr float k_gizmoOperationButtonCount    = 3.0F;
        static constexpr float k_gizmoOperationButtonGapCount = 2.0F;
        static constexpr float k_sameLineOffsetFromStartX     = 0.0F;

        static constexpr bool k_isEnabled = true;
    };
}
