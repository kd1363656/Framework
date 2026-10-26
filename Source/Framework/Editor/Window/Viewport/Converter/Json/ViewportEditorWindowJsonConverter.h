#pragma once

namespace FWK::Editor
{
    class ViewportEditorWindow;
}

namespace FWK::Converter
{
    class ViewportEditorWindowJsonConverter final
    {
    public:

         ViewportEditorWindowJsonConverter() = default;
        ~ViewportEditorWindowJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Editor::ViewportEditorWindow& a_viewportEditorWindow) const;

        nlohmann::json Serialize(const Editor::ViewportEditorWindow& a_viewportEditorWindow) const;

    private:

        static constexpr std::string_view k_gizmoOperationJsonKey         = "GizmoOperation";
        static constexpr std::string_view k_cameraPreviewPanelSizeJsonKey = "CameraPreviewPanelSize";
        static constexpr std::string_view k_debugCameraJsonKey            = "DebugCamera";
    };
}
