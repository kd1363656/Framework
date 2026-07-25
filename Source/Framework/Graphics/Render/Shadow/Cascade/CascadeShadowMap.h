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

		TypeAlias::DescriptorIndex FFetchVALCascadeDSVDescriptorIndex(const UINT a_cascadeIndex) const;

		static constexpr DXGI_FORMAT k_defaultResourceFormat = DXGI_FORMAT_R32_TYPELESS;
		static constexpr DXGI_FORMAT k_defaultDSVFormat      = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT k_defaultSRVFormat      = DXGI_FORMAT_R32_FLOAT;

		static constexpr UINT16 k_defaultCascadeCount = 4U;

		static constexpr UINT k_defaultResolution = 2048U;

	private:

		static constexpr UINT k_requiredSampleCount = 1U;
		static constexpr UINT k_shadowMapMIPSlice   = 0U;

		static constexpr UINT16 k_requiredMIPLevelCount = 1U;

		DepthStencilTexture m_depthStencilTexture = {};

		Converter::CascadeShadowMapJsonConverter m_jsonConverter = {};

		Struct::DepthStencilTextureSettings m_depthStencilTextureSettings = {};
		
		UINT m_resolution = k_defaultResolution;
	};
}