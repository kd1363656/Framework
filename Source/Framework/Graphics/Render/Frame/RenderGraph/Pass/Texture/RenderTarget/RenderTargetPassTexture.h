#pragma once

namespace FWK::Graphics
{
	class RenderTargetPassTexture final : public PassTextureBase
	{
	public:

		 RenderTargetPassTexture()		    = default;
		~RenderTargetPassTexture() override = default;

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

		const auto& GetREFRenderTargetTexture() const { return m_renderTargetTexture; }

		const auto& GetREFClearColor() const { return m_clearColor; }

		auto& GetMutableREFRenderTargetTexture() { return m_renderTargetTexture; }

		auto GetVALFormat() const { return m_format; }

	private:

		RenderTargetTexture m_renderTargetTexture = {};

		Converter::RenderTargetPassTextureJsonConverter m_jsonConverter = {};

		TypeAlias::Math::Color m_clearColor = Constant::k_defaultBackBufferClearColor;

		DXGI_FORMAT m_format = Constant::k_defaultRenderTargetTextureFormat;
	};
}