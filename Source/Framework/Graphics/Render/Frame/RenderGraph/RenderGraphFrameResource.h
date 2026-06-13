#pragma once

namespace FWK::Graphics
{
	class RenderGraphFrameResource final
	{
	private:

		using RenderTargetPassTextureMap = std::unordered_map<Enum::RenderGraphResourceType, std::weak_ptr<RenderTargetPassTexture>>;

	public:

		 RenderGraphFrameResource() = default;
		~RenderGraphFrameResource() = default;

		RenderGraphFrameResource(const RenderGraphFrameResource&)			= delete;
		RenderGraphFrameResource(	  RenderGraphFrameResource&&) noexcept = default;
		
		RenderGraphFrameResource& operator=(const RenderGraphFrameResource&)			  = delete;
		RenderGraphFrameResource& operator=(		 RenderGraphFrameResource&&) noexcept = default;

		void Deserialzie(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Create(const Device&					    a_device,
					const GPUMemoryAllocator&		    a_gpuMemoryAllocator,
					const Struct::ClientSize&		    a_clientSize,
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
						  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool);

		bool Resize(const Device&						a_device,
					const GPUMemoryAllocator&		    a_gpuMemoryAllocator,
					const Struct::ClientSize&		    a_clientSize,
					const UINT64&						a_retiredFenceValue,
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
						  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
						  ResourceReleaseContext&		a_resourceReleaseContext);

		void AddRenderTArgetPassTexture(const std::shared_ptr<RenderTargetPassTexture>& a_renderTargetTexture);

		std::weak_ptr<RenderTargetPassTexture> FindVALRenderTargetPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const;

		const auto& GetREFRenderTargetPassTextureList() const { return m_renderTargetPassTextureList; }

	private:

		RenderTargetPassTextureMap m_renderTargetTextureMap = {};
	
		std::vector<std::shared_ptr<RenderTargetPassTexture>> m_renderTargetPassTextureList = {};

		Converter::RenderGraphFrameResourceJsonConverter m_jsonConverter = {};
	};
}