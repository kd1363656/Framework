#pragma once

namespace FWK::Graphics
{
    // Hi-Z(階層深度バッファ)を作成・保持するクラス
    //
    // 深度バッファを「範囲内で一番奥の深度」を残しながら半分ずつ縮小したテクスチャ(ミップマップ)を作る。
    // 物体が画面上で覆う範囲に合ったミップを数ピクセル読むだけで、
    // 「その物体が手前の物に完全に隠れているか」を判定できる(オクルージョンカリング)
    //
    // 大きさは深度バッファ以下の2の累乗に切り捨てる(縮小のたびにちょうど半分になるようにするため)
    class HierarchicalDepthTexture final
    {
    public:

         HierarchicalDepthTexture() = default;
        ~HierarchicalDepthTexture();

        HierarchicalDepthTexture(const HierarchicalDepthTexture&) = delete;
        HierarchicalDepthTexture(      HierarchicalDepthTexture&&) = delete;

        HierarchicalDepthTexture& operator=(const HierarchicalDepthTexture&) = delete;
        HierarchicalDepthTexture& operator=(      HierarchicalDepthTexture&&) = delete;

        // 深度バッファからHi-Zを作る命令を積む
        // 深度バッファの大きさが変わっていれば、Hi-Zを作り直してから作成する
        bool Build(Renderer& a_renderer, DepthStencilTexture& a_depthStencilTexture);

        bool IsValid() const { return m_isBuilt; }

        auto GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

        auto GetVALWidth   () const { return m_width; }
        auto GetVALHeight  () const { return m_height; }
        auto GetVALMIPCount() const { return m_mipCount; }

    private:

        bool Create(const Renderer& a_renderer, const UINT a_sourceWidth, const UINT a_sourceHeight);

        void Release();

        void TransitionResource(const DirectCommandList& a_directCommandList, const D3D12_RESOURCE_STATES a_afterState);

        static UINT FloorPowerOfTwo(const UINT a_value);

        static constexpr DXGI_FORMAT k_hierarchicalDepthFormat = DXGI_FORMAT_R32_FLOAT;

        static constexpr D3D12_RESOURCE_STATES k_initialResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        // Hi-Zのミップの最大数(4096 × 4096まで対応)
        static constexpr UINT64 k_maxMIPCount = 13ULL;

        static constexpr UINT k_invalidSize                       = 0U;
        static constexpr UINT k_minSize                           = 1U;
        static constexpr UINT k_firstMIPLevel                     = 0U;
        static constexpr UINT k_singleTextureArraySize            = 1U;
        static constexpr UINT k_nonMultisampleCount               = 1U;
        static constexpr UINT k_nonMultisampleQuality             = 0U;
        static constexpr UINT k_singleThreadGroupCount            = 1U;
        static constexpr UINT k_trueValue                         = 1U;
        static constexpr UINT k_falseValue                        = 0U;

        // HLSL側(HierarchicalDepthBuild_CS.hlsl)の1Groupあたりのスレッド数(縦横とも)と同じ値にする
        static constexpr UINT k_hierarchicalDepthBuildThreadCount = 8U;

        Struct::GPUResource m_gpuResource = {};

        std::vector<TypeAlias::DescriptorIndex> m_uavDescriptorIndexList = {};

        // Hi-Z作成用の定数バッファ(フレームリソースごと、ミップ1段につき256byte)
        std::vector<UploadBuffer> m_buildPassConstantBufferList = {};

        TypeAlias::DescriptorIndex m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

        D3D12_RESOURCE_STATES m_currentResourceState = k_initialResourceState;

        UINT m_width        = k_invalidSize;
        UINT m_height       = k_invalidSize;
        UINT m_mipCount     = k_invalidSize;
        UINT m_sourceWidth  = k_invalidSize;
        UINT m_sourceHeight = k_invalidSize;

        bool m_isBuilt = false;
    };
}