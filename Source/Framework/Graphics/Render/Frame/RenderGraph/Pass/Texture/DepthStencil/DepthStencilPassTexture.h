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
					const Window::ClientSize&           a_clientSize,
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool);

		bool Resize(const Device&                       a_device,
					const GPUMemoryAllocator&           a_gpuMemoryAllocator,
					const Window::ClientSize&           a_clientSize,
					const UINT64&						a_retiredFenceValue,
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
						  ResourceReleaseContext&	    a_resourceReleaseContext);

		void SetDepthStencilTextureSettings(const Struct::DepthStencilTextureSettings& a_set) { m_depthStencilTextureSettings = a_set; }

		void SetRenderGraphDepthStencilType(const Enum::RenderGraphDepthStencilType a_set) { m_renderGraphDepthStencilType = a_set; }

		const auto& GetREFDepthStencilTexture() const { return m_depthStencilTexture; }

		const auto& GetREFDepthStencilTextureSettings() const { return m_depthStencilTextureSettings; }

		auto& GetMutableREFDepthStencilTexture() { return m_depthStencilTexture; }

		auto GetVALRenderGraphDepthStencilType() const { return m_renderGraphDepthStencilType; }

	private:

		DepthStencilTexture m_depthStencilTexture = {};

		Converter::DepthStencilPassTextureJsonConverter m_jsonConverter = {};

		Struct::DepthStencilTextureSettings m_depthStencilTextureSettings = {};
		
		Enum::RenderGraphDepthStencilType m_renderGraphDepthStencilType = Enum::RenderGraphDepthStencilType::Invalid;
	};
}