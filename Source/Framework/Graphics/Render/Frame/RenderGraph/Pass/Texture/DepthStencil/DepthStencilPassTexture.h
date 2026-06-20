#pragma once

namespace FWK::Graphics
{
	class DepthStencilPassTexture final
	{
	public:

		 DepthStencilPassTexture() = default;
		~DepthStencilPassTexture() = default;

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

		void SetDepthClearValue(const FLOAT a_set) { m_depthClearValue = a_set; }

		void SetStencilClearValue(const UINT8 a_set) { m_stencilClearValue = a_set; }

		void SetRenderGraphResourceType(const Enum::RenderGraphResourceType a_set) { m_renderGraphResourceType = a_set; }

		void SetWidth (const UINT a_set) { m_width  = a_set; }
		void SetHeight(const UINT a_set) { m_height = a_set; }

		void SetIsFixedSize			   (const bool a_set) { m_isFixedSize			  = a_set; }
		void SetIsSkipClearOnBeginFrame(const bool a_set) { m_isSkipClearOnBeginFrame = a_set; }

		const auto& GetREFDepthStencilTexture() const { return m_depthStencilTexture; }

		auto& GetMutableREFDepthStencilTexture() { return m_depthStencilTexture; }

		auto GetVALFormat() const { return m_format; }

		auto GetVALDepthClearValue() const { return m_depthClearValue; }

		auto GetVALStencilClearValue() const { return m_stencilClearValue; }

		auto GetVALRenderGraphResourceType() const { return m_renderGraphResourceType; }

		auto GetVALWidth () const { return m_width; }
		auto GetVALHeight() const { return m_height; }

		bool GetVALIsFixedSize		      () const { return m_isFixedSize; }
		bool GetVALIsSkipClearOnBeginFrame() const { return m_isSkipClearOnBeginFrame; }

	private:

		DepthStencilTexture m_depthStencilTexture = {};

		Converter::DepthStencilPassTextureJsonConverter m_jsonConverter = {};

		DXGI_FORMAT m_format = Constant::k_defaultDepthStencilTextureFormat;

		FLOAT m_depthClearValue = Constant::k_defaultDepthClearValue;

		UINT8 m_stencilClearValue = Constant::k_defaultStencilClearValue;

		Enum::RenderGraphResourceType m_renderGraphResourceType = Enum::RenderGraphResourceType::Invalid;

		UINT m_width  = Constant::k_emptyTextureWidth;
		UINT m_height = Constant::k_emptyTextureHeight;

		bool m_isFixedSize			   = false;
		bool m_isSkipClearOnBeginFrame = false;
	};
}