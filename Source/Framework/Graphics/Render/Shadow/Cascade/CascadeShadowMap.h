#pragma once

namespace FWK::Graphics
{
    class CascadeShadowMap final
    {
    private:

        struct CascadeData final
        {
            Struct::CBModelCascadeShadowPass m_cbModelCascadeShadowPass = {};

            TypeAlias::Math::Vector3 m_renderedCascadeCenter  = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_renderedLightDirection = TypeAlias::Math::Vector3::Zero;

            float m_splitDepth            = k_initialSplitDepth;
            float m_renderedCascadeRadius = k_initialRadius;

            bool m_isRenderRequired = true;

            bool m_hasRendered = false;
        };

    public:

         CascadeShadowMap() = default;
        ~CascadeShadowMap() = default;

        CascadeShadowMap(const CascadeShadowMap&)           = delete;
        CascadeShadowMap(      CascadeShadowMap&&) noexcept = default;

        CascadeShadowMap& operator=(const CascadeShadowMap&)           = delete;
        CascadeShadowMap& operator=(      CascadeShadowMap&&) noexcept = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        bool Create(const Device&                       a_device,
                    const GPUMemoryAllocator&           a_gpuMemoryAllocator,
                    TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool,
                    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        bool Update();

        nlohmann::json Serialize() const;

        void SetDepthStencilTextureSettings(const Struct::DepthStencilTextureSettings& a_set) { m_depthStencilTextureSettings = a_set; }

        void SetCBCameraPass(const std::shared_ptr<Struct::CBCameraPass>& a_set) { m_cbCameraPass = a_set; }
        void SetCBLightPass (const std::shared_ptr<Struct::CBLightPass>&  a_set) { m_cbLightPass  = a_set; }

        void SetSampleDepthBias  (const float a_set) { m_sampleDepthBias   = a_set; }
        void SetMAXShadowDistance(const float a_set) { m_maxShadowDistance = a_set; }

        void SetResolution(const UINT a_set) { m_resolution = a_set; }

        TypeAlias::DescriptorIndex FetchVALCascadeDSVDescriptorIndex(const UINT a_cascadeIndex) const;

        const Struct::CBModelCascadeShadowPass* FetchPTRModelCascadeShadowPass(const UINT a_cascadeIndex) const;

        // このフレームでシャドウマップを描き直す必要があるか
        // (遠いカスケードは数フレームに1回だけ描き直し、それ以外のフレームは前回の結果を使う)
        bool IsCascadeRenderRequired(const UINT a_cascadeIndex) const;

        const auto& GetREFDepthStencilTextureSettings() const { return m_depthStencilTextureSettings; }

        const auto& GetREFRenderArea() const { return m_renderArea; }

        const auto& GetREFCBCascadeShadowMapPass() const { return m_cbCascadeShadowMapPass; }

        auto& GetMutableREFDepthStencilTexture() { return m_depthStencilTexture; }

        auto GetVALSampleDepthBias  () const { return m_sampleDepthBias; }
        auto GetVALMAXShadowDistance() const { return m_maxShadowDistance; }

        auto GetVALResolution() const { return m_resolution; }

    private:

        bool ShouldRenderCascade(const std::size_t              a_cascadeIndex,
                                 const TypeAlias::Math::Vector3& a_cascadeCenter,
                                 const float                     a_cascadeRadius,
                                 const TypeAlias::Math::Vector3& a_lightDirection) const;

        static constexpr std::size_t k_frustumCornerCount = 8ULL;

        // カスケードごとの描き直しの間隔(フレーム数)と、描き直すフレームのずらし量
        // 近いカスケードは毎フレーム、遠いカスケードは影の細かさが目立たないため間隔を空ける
        // (カスケード2は偶数フレーム、カスケード3は4フレームに1回の奇数フレームで描き、同じフレームに重ならないようにする)
        static constexpr std::array<UINT64, Constant::k_cascadeShadowMapDefaultMAXCascadeCount> k_cascadeRenderIntervalList = { 1ULL, 1ULL, 2ULL, 4ULL };
        static constexpr std::array<UINT64, Constant::k_cascadeShadowMapDefaultMAXCascadeCount> k_cascadeRenderPhaseList    = { 0ULL, 0ULL, 0ULL, 1ULL };

        static constexpr UINT64 k_everyFrameRenderInterval = 1ULL;
        static constexpr UINT64 k_initialFrameCount        = 0ULL;

        static constexpr float k_cascadeReuseMAXMoveRatio = 0.05F;

        static constexpr float k_lightDirectionSameDotThreshold = 0.9999F;

        static constexpr float k_cascadeRadiusChangeEpsilon = 0.001F;

        static constexpr std::array<TypeAlias::Math::Vector3, k_frustumCornerCount> k_ndcFrustumCornerList = { TypeAlias::Math::Vector3{ -1.0F, -1.0F, 0.0F },
                                                                                                               TypeAlias::Math::Vector3{  1.0F, -1.0F, 0.0F },
                                                                                                               TypeAlias::Math::Vector3{ -1.0F,  1.0F, 0.0F },
                                                                                                               TypeAlias::Math::Vector3{  1.0F,  1.0F, 0.0F },
                                                                                                               TypeAlias::Math::Vector3{ -1.0F, -1.0F, 1.0F },
                                                                                                               TypeAlias::Math::Vector3{  1.0F, -1.0F, 1.0F },
                                                                                                               TypeAlias::Math::Vector3{ -1.0F,  1.0F, 1.0F },
                                                                                                               TypeAlias::Math::Vector3{  1.0F,  1.0F, 1.0F } };

        static constexpr float k_cascadeSplitLambda = 0.5F;
        static constexpr float k_fullSplitWeight    = 1.0F;

        static constexpr float k_invalidClipDistance = 0.0F;
        static constexpr float k_initialSplitRatio   = 0.0F;
        static constexpr float k_initialSplitDepth   = 0.0F;
        static constexpr float k_minSampleDepthBias  = 0.0F;

        static constexpr float k_lightViewDistanceScale = 2.0F;
        static constexpr float k_lightViewDepthPadding  = 10.0F;

        static constexpr float k_parallelUpDotThreshold = 0.99F;

        static constexpr float k_initialRadius                  = 0.0F;
        static constexpr float k_cascadeRadiusQuantizationScale = 16.0F;

        static constexpr float k_orthographicDiameterScale      = 2.0F;
        static constexpr float k_cascadeRadiusPaddingTexelCount = 1.0F;
        
        static constexpr std::size_t k_cascadeNumberOffset     = 1ULL;
        static constexpr std::size_t k_frustumPlaneCornerCount = 4ULL;

        static constexpr UINT k_invalidCascadeCount = 0U;

        static constexpr UINT k_requiredSampleCount = 1U;
        static constexpr UINT k_shadowMapMIPSlice   = 0U;

        static constexpr UINT16 k_requiredMIPLevelCount = 1U;

        std::vector<CascadeData> m_cascadeDataList = {};

        std::weak_ptr<Struct::CBCameraPass> m_cbCameraPass = {};
        std::weak_ptr<Struct::CBLightPass>  m_cbLightPass  = {};

        DepthStencilTexture m_depthStencilTexture = {};

        RenderArea m_renderArea = {};

        Converter::CascadeShadowMapJsonConverter m_jsonConverter = {};

        Struct::DepthStencilTextureSettings m_depthStencilTextureSettings = { Constant::k_cascadeShadowMapDefaultResourceFormat,
                                                                              Constant::k_cascadeShadowMapDefaultDSVFormat,
                                                                              Constant::k_cascadeShadowMapDefaultSRVFormat,
                                                                              Constant::k_defaultDepthClearValue,
                                                                              Constant::k_cascadeShadowMapDefaultMAXCascadeCount,
                                                                              k_requiredMIPLevelCount,
                                                                              k_requiredSampleCount,
                                                                              Constant::k_defaultSampleQuality,
                                                                              Constant::k_defaultStencilClearValue };

        Struct::CBCascadeShadowMapPass m_cbCascadeShadowMapPass = {};

        float m_sampleDepthBias   = Constant::k_cascadeShadowMapDefaultSampleDepthBias;
        float m_maxShadowDistance = Constant::k_cascadeShadowMapDefaultMAXShadowDistance;

        UINT64 m_updateFrameCount = k_initialFrameCount;

        UINT m_resolution = Constant::k_cascadeShadowMapDefaultResolution;
    };
}