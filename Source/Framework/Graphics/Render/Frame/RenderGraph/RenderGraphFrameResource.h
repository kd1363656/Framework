#pragma once

namespace FWK::Graphics
{
	class RenderGraphFrameResource final
	{
	private:

		using RenderTargetPassTextureMap = std::unordered_map<Enum::RenderGraphResourceType, std::weak_ptr<RenderTargetPassTexture>>;
		using DepthStencilPassTextureMap = std::unordered_map<Enum::RenderGraphResourceType, std::weak_ptr<DepthStencilPassTexture>>;

	public:

		 RenderGraphFrameResource() = default;
		~RenderGraphFrameResource() = default;

		RenderGraphFrameResource(const RenderGraphFrameResource&)			= delete;
		RenderGraphFrameResource(	  RenderGraphFrameResource&&) noexcept = default;
		
		RenderGraphFrameResource& operator=(const RenderGraphFrameResource&)			  = delete;
		RenderGraphFrameResource& operator=(		 RenderGraphFrameResource&&) noexcept = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Create(const Device&			  a_device,
					const GPUMemoryAllocator& a_gpuMemoryAllocator,
					const Struct::ClientSize& a_clientSize,
						  ResourceContext&    a_resourceContext) const;

		bool Resize(const Device&			  a_device,
					const GPUMemoryAllocator& a_gpuMemoryAllocator,
					const Struct::ClientSize& a_clientSize,
					const UINT64&			  a_retiredFenceValue,
						  ResourceContext&    a_resourceContext) const;

		void AddRenderTargetPassTexture(const std::shared_ptr<RenderTargetPassTexture>& a_renderTargetPassTexture);
		void AddDepthStencilPassTexture(const std::shared_ptr<DepthStencilPassTexture>& a_depthStencilPassTexture);

		std::weak_ptr<RenderTargetPassTexture> FindVALRenderTargetPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const;
		std::weak_ptr<DepthStencilPassTexture> FindVALDepthStencilPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const;

		const auto& GetREFRenderTargetPassTextureList() const { return m_renderTargetPassTextureList; }
		const auto& GetREFDepthStencilPassTextureList() const { return m_depthStencilPassTextureList; }

	private:

		RenderTargetPassTextureMap m_renderTargetPassTextureMap = {};
		DepthStencilPassTextureMap m_depthStencilPassTextureMap = {};

		std::vector<std::shared_ptr<RenderTargetPassTexture>> m_renderTargetPassTextureList = {};
		std::vector<std::shared_ptr<DepthStencilPassTexture>> m_depthStencilPassTextureList = {};

		Converter::RenderGraphFrameResourceJsonConverter m_jsonConverter = {};
	};
}