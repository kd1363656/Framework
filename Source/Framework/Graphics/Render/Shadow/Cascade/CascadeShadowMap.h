#pragma once

namespace FWK::Graphics
{
	class CascadeShadowMap
	{
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

		nlohmann::json Serialize() const;

		void SetDepthStencilTextureSettings(const Struct::DepthStencilTextureSettings& a_set) { m_depthStencilTextureSettings = a_set; }

		void SetResolution(const UINT a_set) { m_resolution = a_set; }

		TypeAlias::DescriptorIndex FetchVALCascadeDSVDescriptorIndex(const UINT a_cascadeIndex) const;

		const auto& GetREFDepthStencilTextureSettings() const { return m_depthStencilTextureSettings; }

		auto GetVALResolution() const { return m_resolution; }

	private:

		static constexpr UINT k_requiredSampleCount = 1U;
		static constexpr UINT k_shadowMapMIPSlice   = 0U;

		static constexpr UINT16 k_requiredMIPLevelCount = 1U;

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