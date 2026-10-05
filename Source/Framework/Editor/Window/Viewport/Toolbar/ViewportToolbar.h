#pragma once

namespace FWK::Editor
{
    class EditorCamera;
    class ViewportEditorWindow;
};

namespace FWK::Editor
{
    class ViewportToolbar final
    {
    public:

         ViewportToolbar() = default;
        ~ViewportToolbar() = default;

        void Draw(ViewportEditorWindow& a_viewportEditorWindow);

    private:

        void DrawDisplayOptionsButton(ViewportEditorWindow& a_viewportEditorWindow);
        void DrawDisplayOptionsPopup (ViewportEditorWindow& a_viewportEditorWindow);
        void DrawCollisionMenuItem   () const;
        void DrawFrustumMenuItem     (ViewportEditorWindow& a_viewportEditorWindow) const;
        void DrawCulledResultMenuItem(ViewportEditorWindow& a_viewportEditorWindow) const;

        void DrawCameraButton(ViewportEditorWindow& a_viewportEditorWindow);
        void DrawCameraPopup (EditorCamera&         a_editorCamera);

        static constexpr std::string_view k_toolbarChildID                = "##ViewportToolbar";
        static constexpr std::string_view k_displayOptionsPopupID         = "##SceneViewDisplayOptionsPopup";
        static constexpr std::string_view k_cameraOptionsPopupID          = "##ViewportCameraOptionsPopup";
        static constexpr std::string_view k_collisionMenuItemLabel        = "当たり判定の可視化";
        static constexpr std::string_view k_drawFrustumMenuItemLabel      = "視錐台の可視化";
        static constexpr std::string_view k_drawCulledResultMenuItemLabel = "カリング結果の可視化";

        static constexpr std::string_view k_displayOptionsButtonLabel = "表示";

        static constexpr std::string_view k_cameraOptionsButtonLabel  = "\xEF\x80\xB0 \xEF\x83\x97";

        static constexpr std::string_view k_fovYDegreeLabel  = "視野角";
        static constexpr std::string_view k_moveSpeedLabel   = "移動速度";
        static constexpr std::string_view k_rotateSpeedLabel = "回転速度";

        static constexpr float k_toolbarHeight     = 36.0F;
        static constexpr float k_toolbarButtonSize = 28.0F;

        static constexpr float k_fovYDegreeMIN = 10.0F;
        static constexpr float k_fovYDegreeMAX = 170.0F;

        static constexpr float k_moveSpeedMIN = 0.1F;
        static constexpr float k_moveSpeedMAX = 100.0F;

        static constexpr float k_rotateSpeedMIN       = 0.0001F;
        static constexpr float k_rotateSpeedMAX       = 0.05F;
        static constexpr float k_rotateSpeedDragValue = 0.0005F;
    };
}