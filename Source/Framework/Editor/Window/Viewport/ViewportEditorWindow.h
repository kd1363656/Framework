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

        void SetIsDrawFrustum     (const bool a_set) { m_isDrawFrustum      = a_set; }
        void SetIsDrawCulledResult(const bool a_set) { m_isDrawCulledResult = a_set; }

        bool GetVALIsDrawFrustum     () const { return m_isDrawFrustum;      }
        bool GetVALIsDrawCulledResult() const { return m_isDrawCulledResult; }

    private:

        ImTextureID FetchVALViewportTextureID() const;

        void DrawViewportTexture(const ImVec2& a_viewportSize, const ImTextureID& a_textureID) const;

        void DrawCameraPreview() const;

        void RequestCameraPreview(const EditorManager& a_editorManager, const ImVec2& a_viewportSize) const;

        void RegisterDebugCamera     () const;
        void UpdateEditorCameraInput ();

        void ReleaseViewportTextureDescriptors();

        static constexpr std::string_view k_editorName                 = "ビューポート";
        static constexpr std::string_view k_thisWindowExplanationLabel = "現在のシーンの描画状態を見ることができるウィンドウ。";

        static constexpr float k_minViewportSize = 1.0F;

        static constexpr float k_cameraPreviewWidthRatio = 0.25F;

        static constexpr float k_cameraPreviewMargin          = 8.0F;
        static constexpr float k_cameraPreviewBorderThickness = 2.0F;
        static constexpr float k_cameraPreviewBorderRounding  = 0.0F;

        static constexpr ImDrawFlags k_cameraPreviewBorderFlags = ImDrawFlags_None;

        static constexpr float k_viewportUVMINX = 0.0F;
        static constexpr float k_viewportUVMINY = 0.0F;
        static constexpr float k_viewportUVMAXX = 1.0F;
        static constexpr float k_viewportUVMAXY = 1.0F;
        
        static constexpr ImTextureID k_invalidViewportTextureID = {};

        std::vector<TypeAlias::DescriptorIndex> m_imGuiSRVDescriptorIndexList;
        std::vector<TypeAlias::DescriptorIndex> m_previewImGuiSRVDescriptorIndexList;

        std::unique_ptr<EditorCamera> m_editorCamera;

        ViewportToolbar m_toolbar;

        Converter::ViewportEditorWindowJsonConverter m_jsonConverter;

        bool m_isDrawFrustum      = false;
        bool m_isDrawCulledResult = false;

        FWK_DEFINE_TYPE_INFO(ViewportEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ViewportEditorWindow)