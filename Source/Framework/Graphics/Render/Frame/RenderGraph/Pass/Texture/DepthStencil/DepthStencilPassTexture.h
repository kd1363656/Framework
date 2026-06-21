#pragma once

namespace FWK::Graphics
{
	class DepthStencilPassTexture final : public PassTextureBase
	{
	public:

		 DepthStencilPassTexture()          = default;
		~DepthStencilPassTexture() override = default;

		DepthStencilPassTexture(const DepthStencilPassTexture&)			  = delete;
		DepthStencilPassTexture(	  DepthStencilPassTexture&&) noexcept = default;
		
		DepthStencilPassTexture& operator=(const DepthStencilPassTexture&)			 = delete;
		DepthStencilPassTexture& operator=(		 DepthStencilPassTexture&&) noexcept = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Create(const Device&                       a_device,
					const GPUMemoryAllocator&           a_gpuMemoryAllocator,
					const Struct::ClientSize&           a_clientSize,	
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool);

		bool Resize(const Device&                       a_device,
					const GPUMemoryAllocator&           a_gpuMemoryAllocator,
					const Struct::ClientSize&           a_clientSize,	
					const UINT64&						a_retiredFenceValue,
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
						  ResourceReleaseContext&	    a_resourceReleaseContext);

		void SetFormat(const DXGI_FORMAT a_set) { m_format = a_set; }

		void SetRenderGraphDepthStencilType(const Enum::RenderGraphDepthStencilType a_set) { m_renderGraphDepthStencilType = a_set; }

		void SetDepthClearValue(const FLOAT a_set) { m_depthClearValue = a_set; }

		void SetStencilClearValue(const UINT8 a_set) { m_stencilClearValue = a_set; }

		const auto& GetREFDepthStencilTexture() const { return m_depthStencilTexture; }

		auto& GetMutableREFDepthStencilTexture() { return m_depthStencilTexture; }

		auto GetVALFormat() const { return m_format; }

		auto GetVALRenderGraphDepthStencilType() const { return m_renderGraphDepthStencilType; }

		auto GetVALDepthClearValue() const { return m_depthClearValue; }

		auto GetVALStencilClearValue() const { return m_stencilClearValue; }

	private:

		DepthStencilTexture m_depthStencilTexture = {};

		Converter::DepthStencilPassTextureJsonConverter m_jsonConverter = {};

		DXGI_FORMAT m_format = Constant::k_defaultDepthStencilTextureFormat;

		Enum::RenderGraphDepthStencilType m_renderGraphDepthStencilType = Enum::RenderGraphDepthStencilType::Invalid;

		FLOAT m_depthClearValue = Constant::k_defaultDepthClearValue;

		UINT8 m_stencilClearValue = Constant::k_defaultStencilClearValue;
	};
}