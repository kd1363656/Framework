#pragma once

namespace FWK
{
    namespace Graphics
    {
        class Camera;
    }

    namespace Editor
    {
        class EditorCamera;
    }
};

namespace FWK::Editor
{
    class ViewportEditorWindow final : public EditorWindowBase
    {
    public:

         ViewportEditorWindow();
        ~ViewportEditorWindow() override;

        void Deserialize    (const nlohmann::json& a_rootJson) override;
        void PostDeserialize()                                 override;

        void Draw(EditorManager& a_editorManager) override;

        nlohmann::json Serialize() override;

        void SetupViewportTextureDescriptors();

        const auto& GetREFEditorCamera() const { return m_editorCamera; }

        auto& GetMutableREFEditorCamera() { return m_editorCamera; }

    private:

        ImTextureID FetchVALViewportTextureID() const;

        void DrawViewportTexture(const ImTextureID& a_textureID, const ImVec2& a_viewportSize) const;

        void RegisterEditorCamera    () const;
        void UpdateEditorCameraInput ();

        static constexpr std::string_view k_editorName                 = "ビューポート";
        static constexpr std::string_view k_thisWindowExplanationLabel = "現在のシーンの描画状態を見ることができるウィンドウ。";

        static constexpr float k_minViewportSize = 1.0F;

        static constexpr float k_viewportUVMINX = 0.0F;
        static constexpr float k_viewportUVMINY = 0.0F;
        static constexpr float k_viewportUVMAXX = 1.0F;
        static constexpr float k_viewportUVMAXY = 1.0F;
        
        static constexpr ImTextureID k_invalidViewportTextureID = {};

        std::vector<TypeAlias::DescriptorIndex> m_imGuiSRVDescriptorIndexList;

        std::unique_ptr<EditorCamera> m_editorCamera;

        Converter::ViewportEditorWindowJsonConverter m_jsonConverter;

        ViewportToolbar m_toolbar;

        FWK_DEFINE_TYPE_INFO(ViewportEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ViewportEditorWindow)