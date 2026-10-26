#pragma once

namespace FWK
{
    class GameObjectCameraComponent;
}

namespace FWK::Editor
{
    // 選択中のGameObjectがCameraComponentを持っているとき、そのカメラから見た絵をビューポートの右下に表示するクラス
    //
    // 絵はRendererのRenderViewCaptureに描いてもらう(メインの描画とは別に、1フレームに1回追加で描く)
    // 表示枠は左上の角をドラッグすると大きさを変えられる
    // 表示枠の縦横比に合わせたカメラ(このクラスが持つ、Rendererへ登録しないカメラ)で描くため、
    // 枠を引き伸ばしても映っている物の形は伸び縮みしない(ビューポートと同じ仕組み)
    class ViewportCameraPreview final
    {
    public:

         ViewportCameraPreview();
        ~ViewportCameraPreview();

        void Draw(const std::weak_ptr<GameObjectCameraComponent>& a_cameraComponent, const Struct::ViewportScreenRect& a_viewportRect);

        void SetPanelSize(const ImVec2& a_set) { m_panelSize = a_set; }

        // マウスが表示枠の上にあるか(表示枠の上のクリックで、後ろの物を選ばないようにするため)
        bool FetchVALIsMouseOverPanel() const;

        const auto& GetREFPanelSize() const { return m_panelSize; }

    private:

        void UpdatePreviewCamera(const GameObjectCameraComponent& a_cameraComponent, const float a_aspectRatio);

        void DrawResizeHandle(const ImVec2& a_viewportSize);

        bool FetchVALCaptureTextureID(ImTextureID& a_outTextureID);

        static constexpr std::string_view k_resizeHandleLabel = "##ViewportCameraPreviewResizeHandle";
        static constexpr std::string_view k_waitingLabel      = "プレビューを準備しています...";

        static constexpr ImVec2 k_minPanelSize      = { 120.0F, 68.0F };
        static constexpr ImVec2 k_resizeHandleSize  = { 14.0F, 14.0F };
        static constexpr ImVec2 k_titlePadding      = { 6.0F, 4.0F };

        static constexpr float k_panelMargin          = 12.0F;
        static constexpr float k_panelBorderThickness = 2.0F;
        static constexpr float k_panelRounding        = 0.0F;

        static constexpr ImU32 k_panelBackgroundColor = IM_COL32(16, 16, 16, 230);
        static constexpr ImU32 k_panelBorderColor     = IM_COL32(0, 112, 224, 255);
        static constexpr ImU32 k_titleBackgroundColor = IM_COL32(0, 0, 0, 160);
        static constexpr ImU32 k_titleTextColor       = IM_COL32(255, 255, 255, 255);
        static constexpr ImU32 k_resizeHandleColor    = IM_COL32(0, 112, 224, 255);

        Graphics::Camera m_previewCamera;

        ImVec2 m_panelSize;
        ImVec2 m_panelMin;
        ImVec2 m_panelMax;

        std::uint64_t m_copiedCaptureTextureVersion;

        TypeAlias::DescriptorIndex m_imGuiSRVDescriptorIndex;

        bool m_isPreviewCameraSetup;
        bool m_isPanelVisible;
    };
}
