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

        static constexpr std::string_view k_editorCameraJsonKey       = "EditorCamera";
        static constexpr std::string_view k_isDrawFrustumJsonKey      = "IsDrawFrustum";
        static constexpr std::string_view k_isDrawCulledResultJsonKey = "IsDrawCulledResult";
    };
}