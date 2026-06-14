#pragma once

namespace FWK::Graphics
{
	class RenderTargetPassTexture final
	{
	public:

		 RenderTargetPassTexture() = default;
		~RenderTargetPassTexture() = default;

		RenderTargetPassTexture(const RenderTargetPassTexture&)			  = delete;
		RenderTargetPassTexture(	  RenderTargetPassTexture&&) noexcept = default;
		
		RenderTargetPassTexture& operator=(const RenderTargetPassTexture&)			 = delete;
		RenderTargetPassTexture& operator=(		 RenderTargetPassTexture&&) noexcept = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Create(const Device&						a_device,
					const GPUMemoryAllocator&			a_gpuMemoryAllocator,
					const Struct::ClientSize&			a_clientSize,
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
						  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool);

		bool Resize(const Device&					    a_device,
					const GPUMemoryAllocator&		    a_gpuMemoryAllocator,
					const Struct::ClientSize&		    a_clientSize,
					const UINT64&					    a_retiredFenceValue,
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
						  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
						  ResourceReleaseContext&		a_resourceReleaseContext);

		void SetClearColor(const TypeAlias::Math::Color& a_set) { m_clearColor = a_set; }

		void SetFormat(const DXGI_FORMAT a_set) { m_format = a_set; }

		void SetRenderGraphResourceType(const Enum::RenderGraphResourceType a_set) { m_renderGraphResourceType = a_set; }

		void SetWidth (const UINT a_set) { m_width  = a_set; }
		void SetHeight(const UINT a_set) { m_height = a_set; }

		void SetIsFixedSize(const bool a_set) { m_isFixedSize = a_set; }

		const auto& GetREFRenderTargetTexture() const { return m_renderTargeTexture; }

		const auto& GetREFClearColor() const { return m_clearColor; }

		auto& GetMutableREFRenderTargetTexture() const { return m_renderTargeTexture; }

		auto GetVALFormat() const { return m_format; }

		auto GetVALRenderGraphResourceType() const { return m_renderGraphResourceType; }

		auto GetVALWidth () const { return m_width; }
		auto GetVALHeight() const { return m_height; }

		auto GetVALIsFixedSize() const { return m_isFixedSize; }

	private:

		UINT FetchVALTextureWidth (const UINT a_width)  const;
		UINT FetchVALTextureHeight(const UINT a_height) const;

		RenderTargetTexture m_renderTargeTexture = {};

		Converter::RenderTargetPassTextureJsonConverter m_jsonConverter = {};

		TypeAlias::Math::Color m_clearColor = Constant::k_defaultBackBufferClearColor;

		DXGI_FORMAT m_format = Constant::k_defaultRenderTargetTextureFormat;

		Enum::RenderGraphResourceType m_renderGraphResourceType = Enum::RenderGraphResourceType::Invalid;

		UINT m_width  = Constant::k_emptyTextureWidth;
		UINT m_height = Constant::k_emptyTextureHeight;

		bool m_isFixedSize = false;
	};
}