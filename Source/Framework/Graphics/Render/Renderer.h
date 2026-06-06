#pragma once

namespace FWK::Graphics
{
	class Renderer final
	{
	public:

		 Renderer() = default;
		~Renderer() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool PostDeserialize(const Device&						 a_device, 
							 const Window&						 a_window,
							 const Factory&						 a_factory,
								   TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);
		
		void BeginFrame(const ResourceContext& a_resourceContext);
		void EndFrame  ();

		nlohmann::json Serialize() const;

		void Resize(const Device& a_device, const Struct::ClientSize& a_clientSize, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);

		void AddFrameResource(const std::shared_ptr<FrameResource>& a_frameResource);

		const auto& GetREFFrameResourceList() const { return m_frameResourceList; }
		
		const auto& GetREFSwapChain() const { return m_swapChain; }
		
		const auto& GetREFDirectCommandList() const { return m_directCommandList; }

		auto& GetMutableREFSwapChain() { return m_swapChain; }
		
	private:

		void ResetCommandObjects(const FrameResource& a_frameResource);

		void DecideNextFrameUseFrameResource();

		bool PrepareForSwapChainResize();

		static constexpr std::size_t k_initialFrameResourceIndex   = 0ULL;
		static constexpr std::size_t k_frameResourceIndexIncrement = 1ULL;

		std::vector<std::shared_ptr<FrameResource>> m_frameResourceList = {};

		std::weak_ptr<FrameResource> m_currentFrameResource = {};

		SwapChain m_swapChain = {};

		DirectCommandQueue m_directCommandQueue = {};
		DirectCommandList  m_directCommandList  = {};

		RenderGraph m_renderGraph = {};

		Converter::RendererJsonConverter m_jsonConverter = {};

		std::size_t m_currentFrameResourceIndex = k_initialFrameResourceIndex;
	};
}