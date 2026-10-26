#pragma once

namespace FWK::Graphics
{
    class Renderer;
}

namespace FWK::Graphics
{
    // メインの描画とは別のカメラから見た絵を、1フレームに1回だけ追加で描いてTextureへ写すクラス
    // (エディターで、選択したカメラから見た絵をビューポートの隅に表示するために使う)
    //
    // 流れ :
    // 1. 描いてほしいフレームの前に、Requestでカメラの定数バッファを渡す(依頼は1回描くと消える)
    // 2. Renderer::Executeの最初に、描画カメラとカリングカメラをそのカメラへ差し替えて、RenderGraphを追加で実行する
    //    (影、アニメーション、物理のデバッグ表示、バックバッファへの出力は、メインの描画で行うため飛ばす)
    // 3. 出来上がったFinalColorをこのクラスのTextureへコピーし、差し替えを元に戻してから、メインの描画用にPassTextureをクリアする
    class RenderViewCapture final
    {
    public:

         RenderViewCapture() = default;
        ~RenderViewCapture() = default;

        void BeginFrame();
        void Execute   (const ResourceContext& a_resourceContext, Renderer& a_renderer);

        void Request(const std::weak_ptr<Struct::CBCameraPass>& a_cameraConstantBuffer) { m_requestCameraConstantBuffer = a_cameraConstantBuffer; }

        const auto& GetREFCaptureTexture() const { return m_captureTexture; }

        auto GetVALCaptureTextureVersion() const { return m_captureTextureVersion; }

        bool GetVALIsCapturedThisFrame() const { return m_isCapturedThisFrame; }

    private:

        bool PrepareCaptureTexture(const Renderer& a_renderer, const RenderTargetPassTexture& a_sourcePassTexture);

        void CopyFinalColor(const DirectCommandList& a_directCommandList, RenderTargetTexture& a_sourceTexture);

        void TransitionTexture(const DirectCommandList& a_directCommandList, RenderTargetTexture& a_texture, const D3D12_RESOURCE_STATES a_afterState) const;

        static constexpr std::uint64_t k_initialCaptureTextureVersion = 0ULL;
        static constexpr std::uint64_t k_captureTextureVersionStep    = 1ULL;

        std::weak_ptr<Struct::CBCameraPass> m_requestCameraConstantBuffer = {};

        RenderTargetTexture m_captureTexture = {};

        std::uint64_t m_captureTextureVersion = k_initialCaptureTextureVersion;

        bool m_isCapturedThisFrame = false;
    };
}
