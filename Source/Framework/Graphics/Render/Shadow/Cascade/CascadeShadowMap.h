#pragma once

namespace FWK::Graphics
{
	class CascadeShadowMap
	{
	private:

		struct CascadeData final
		{
			static constexpr float k_initialSplitDepth = 0.0F;

			TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

			float m_splitDepth = k_initialSplitDepth;
		};

	public:

		 CascadeShadowMap() = default;
		~CascadeShadowMap() = default;

		CascadeShadowMap(const CascadeShadowMap&)           = delete;
		CascadeShadowMap(      CascadeShadowMap&&) noexcept = default;

		CascadeShadowMap& operator=(const CascadeShadowMap&)           = delete;
		CascadeShadowMap& operator=(      CascadeShadowMap&&) noexcept = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool Create(const Device&                             a_device,
			        const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
			              TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool,
			              TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		bool Update();

		nlohmann::json Serialize() const;

		void SetDepthStencilTextureSettings(const Struct::DepthStencilTextureSettings& a_set) { m_depthStencilTextureSettings = a_set; }

		void SetCBCameraPass(const std::shared_ptr<Struct::CBCameraPass>& a_set) { m_cbCameraPass = a_set; }
		void SetCBLightPass (const std::shared_ptr<Struct::CBLightPass>&  a_set) { m_cbLightPass  = a_set; }

		void SetResolution(const UINT a_set) { m_resolution = a_set; }

		TypeAlias::DescriptorIndex FetchVALCascadeDSVDescriptorIndex(const UINT a_cascadeIndex) const;

		const auto& GetREFDepthStencilTextureSettings() const { return m_depthStencilTextureSettings; }

		const auto& GetREFRenderArea() const { return m_renderArea; }

		auto& GetMutableREFDepthStencilTexture() { return m_depthStencilTexture; }

		auto GetVALResolution() const { return m_resolution; }

	private:

		static constexpr float k_cascadeSplitLambda = 0.5F;

		static constexpr float k_invalidClipDistance = 0.0F;

		static constexpr float k_lightViewDistanceScale = 2.0F;

		static constexpr float k_lightViewDepthPaddig = 10.0F;

		static constexpr float k_directionLengthSquaredEpsilon = 0.000001F;
		static constexpr float k_parallelUpDotThreshold        = 0.99F;

		static constexpr float k_ndcMINX = -1.0F;
		static constexpr float k_ndcMAXX =  1.0F;
		static constexpr float k_ndcMINY = -1.0F;
		static constexpr float k_ndcMAXY =  1.0F;
		static constexpr float k_ndcNearZ = 0.0F;
		static constexpr float k_ndcFarZ  = 1.0F;

		static constexpr std::size_t k_firstCascadeIndex       = 0ULL;
		static constexpr std::size_t k_cascadeNumberOfset      = 1ULL;
		static constexpr std::size_t k_firstFrustumCornerIndex = 0ULL;
		static constexpr std::size_t k_frustumPlaneCornerCount = 4ULL;
		static constexpr std::size_t k_frustumCornerCount      = 8ULL;

		static constexpr UINT k_requiredSampleCount = 1U;
		static constexpr UINT k_shadowMapMIPSlice   = 0U;

		static constexpr UINT16 k_requiredMIPLevelCount = 1U;

		std::weak_ptr<Struct::CBCameraPass> m_cbCameraPass = {};
		std::weak_ptr<Struct::CBLightPass>  m_cbLightPass  = {};

		DepthStencilTexture m_depthStencilTexture = {};

		RenderArea m_renderArea = {};

		Converter::CascadeShadowMapJsonConverter m_jsonConverter = {};

		Struct::DepthStencilTextureSettings m_depthStencilTextureSettings = { Constant::k_cascadeShadowMapDefaultResourceFormat,
																			  Constant::k_cascadeShadowMapDefaultDSVFormat,
																			  Constant::k_cascadeShadowMapDefaultSRVFormat,
		                                                                      Constant::k_defaultDepthClearValue,
		                                                                      Constant::k_cascadeShadowMapDefaultCascadeCount,
		                                                                      k_requiredMIPLevelCount,
		                                                                      k_requiredSampleCount, 
																			  Constant::k_defaultSampleQuality,
																			  Constant::k_defaultStencilClearValue };
		UINT m_resolution = Constant::k_cascadeShadowMapDefaultResolution;
	};
}