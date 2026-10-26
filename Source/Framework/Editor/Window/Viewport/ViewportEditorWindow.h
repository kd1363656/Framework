#pragma once

namespace FWK
{
    class GameObjectCameraComponent;
}

namespace FWK::Graphics
{
    class Camera;
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

        void OnDrawSkipped() override;

        void SetupViewportTextureDescriptors();

        void SetGizmoOperation(const Enum::ViewportGizmoOperation a_set) { m_gizmoOperation = a_set; }

        const auto& GetREFCameraPreview() const { return m_cameraPreview; }
        const auto& GetREFDebugCamera  () const { return m_debugCamera; }

        auto& GetMutableREFCameraPreview() { return m_cameraPreview; }
        auto& GetMutableREFDebugCamera  () { return m_debugCamera; }

        auto GetVALGizmoOperation() const { return m_gizmoOperation; }

        float GetVALViewportAspectRatio() const { return m_viewportAspectRatio; }

    private:

        ImTextureID FetchVALViewportTextureID() const;

        void DrawViewportTexture(const ImTextureID& a_textureID, const ImVec2& a_viewportSize) const;

        // Textureの上に重ねる物(GameObjectの目印、ギズモ、カメラのプレビュー、ボタン)を描き、クリックでの選択を行う
        void DrawViewportOverlay(EditorManager& a_editorManager, const Struct::ViewportScreenRect& a_viewportRect);

        bool FetchVALIsViewportHovered(const Struct::ViewportScreenRect& a_viewportRect) const;

        std::weak_ptr<GameObjectCameraComponent> FetchVALSelectedCameraComponent(const EditorGameObjectSelectionState& a_gameObjectSelectionState) const;

        static constexpr std::string_view k_editorName                 = "ビューポート";
        static constexpr std::string_view k_thisWindowExplanationLabel = "現在のシーンの描画状態を見ることができるウィンドウ。";

        static constexpr float k_minViewportSize = 1.0F;

        static constexpr float k_viewportUVMINX = 0.0F;
        static constexpr float k_viewportUVMINY = 0.0F;
        static constexpr float k_viewportUVMAXX = 1.0F;
        static constexpr float k_viewportUVMAXY = 1.0F;

        static constexpr ImTextureID k_invalidViewportTextureID = {};

        static constexpr bool k_intersectWithCurrentClipRect = true;

        std::vector<TypeAlias::DescriptorIndex> m_imGuiSRVDescriptorIndexList;

        ViewportToolbar        m_toolbar;
        ViewportOverlayToolbar m_overlayToolbar;
        ViewportDebugCamera    m_debugCamera;
        ViewportCameraPreview  m_cameraPreview;
        ViewportGizmo          m_gizmo;
        ViewportPicker         m_picker;

        Converter::ViewportEditorWindowJsonConverter m_jsonConverter;

        Enum::ViewportGizmoOperation m_gizmoOperation;

        float m_viewportAspectRatio;

        bool m_isViewportHoveredPreviousFrame;

        FWK_DEFINE_TYPE_INFO(ViewportEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ViewportEditorWindow)
