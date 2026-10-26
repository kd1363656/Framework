#pragma once

namespace FWK::Graphics
{
    // カメラの定数バッファをGPUへ送るクラス
    //
    // 描画に使うカメラ(描画カメラ)は、通常はCameraクラスが登録したメインカメラ(SourceConstantBuffer)
    // エディターのデバッグカメラや、選択したカメラのプレビューを描く間だけ、OverrideSourceConstantBufferで一時的に差し替える
    //
    // カリングに使うカメラ(カリングカメラ)は、通常は描画カメラと同じ
    // デバッグカメラで描画している間はメインカメラをカリングカメラにすることで、
    // 「メインカメラがどこまで描いているか(カリングの状況)」をデバッグカメラから見ることができる
    class CameraPassDrawRequest final : public CachedPassConstantBufferDrawRequestBase<Struct::CBCameraPass>
    {
    public:

         CameraPassDrawRequest()          = default;
        ~CameraPassDrawRequest() override = default;

        bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

        void SetOverrideSourceConstantBuffer(const std::weak_ptr<Struct::CBCameraPass>& a_set) { m_overrideSourceConstantBuffer = a_set; }
        void SetCullingSourceConstantBuffer (const std::weak_ptr<Struct::CBCameraPass>& a_set) { m_cullingSourceConstantBuffer  = a_set; }

        void SetIsCullingDebugVisualize(const bool a_set) { m_isCullingDebugVisualize = a_set; }

        // 描画に使うカメラの定数バッファ(差し替え中は差し替え先、そうでなければメインカメラ)
        std::shared_ptr<Struct::CBCameraPass> FetchVALDrawConstantBuffer() const;

        // カリング(視錐台、LOD選択、Hi-Z)に使うカメラの定数バッファ(指定が無ければ描画カメラと同じ)
        std::shared_ptr<Struct::CBCameraPass> FetchVALCullingConstantBuffer() const;

        // オクルージョンカリング(Hi-Z)を行ってよいか
        // Hi-Zは直前のフレームの描画カメラの深度から作られるため、描画カメラを差し替えている間は判定が合わず行えない
        bool FetchVALIsOcclusionCullingAllowed() const { return m_overrideSourceConstantBuffer.expired(); }

        const auto& GetREFOverrideSourceConstantBuffer() const { return m_overrideSourceConstantBuffer; }
        const auto& GetREFCullingSourceConstantBuffer () const { return m_cullingSourceConstantBuffer; }

        bool GetVALIsCullingDebugVisualize() const { return m_isCullingDebugVisualize; }

    private:

        void UpdateCameraConstantBuffer();

        std::weak_ptr<Struct::CBCameraPass> m_overrideSourceConstantBuffer = {};
        std::weak_ptr<Struct::CBCameraPass> m_cullingSourceConstantBuffer  = {};

        bool m_isCullingDebugVisualize = false;

        FWK_DEFINE_TYPE_INFO(CameraPassDrawRequest, DrawRequestPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory , FWK::Graphics::CameraPassDrawRequest)
