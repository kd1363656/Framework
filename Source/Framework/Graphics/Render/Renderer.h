#pragma once

namespace FWK::Graphics
{
	class Renderer final
	{
	private:

		using RootSignatureMap = std::unordered_map<Enum::RootSignatureType, std::shared_ptr<RootSignature>>;
		using PipelineStateMap = std::unordered_map<Enum::PipelineStateType, std::shared_ptr<PipelineState>>;
		
	public:

		 Renderer() = default;
		~Renderer() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool PostDeserialize(const Device&						 a_device, 
							 const Window&						 a_window,
							 const Factory&						 a_factory,
							 const ShaderCompiler&				 a_shaderCompiler,
								   TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);
		
		void BeginFrame(const ResourceContext& a_resourceContext);
		void EndFrame  ();

		nlohmann::json Serialize() const;

		void Resize(const Device&						a_device, 
					const ResourceReleaseContext&		a_resourceReleaseContext,
					const Struct::ClientSize&			a_clientSize, 
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);

		void AddFrameResource(const std::shared_ptr<FrameResource>& a_frameResource);
		void AddRootSignature(const std::shared_ptr<RootSignature>& a_rootSignature, const Enum::RootSignatureType a_rootSignatureType);
		void AddPipelineState(const std::shared_ptr<PipelineState>& a_pipelineState, const Enum::PipelineStateType a_pipelineStateType);

		std::weak_ptr<RootSignature> FindVALRootSignature(const Enum::RootSignatureType a_rootSignatureType) const;
		std::weak_ptr<PipelineState> FindVALPipelineState(const Enum::PipelineStateType a_pipelineStateType) const;

		const auto& GetREFFrameResourceList() const { return m_frameResourceList; }
		const auto& GetREFRootSignatureMap () const { return m_rootSignatureMap; }
		const auto& GetREFPipelineStateMap () const { return m_pipelineStateMap; }

		const auto& GetREFSwapChain  () const { return m_swapChain; }
		const auto& GetREFRenderGraph() const { return m_renderGraph; }
		
		const auto& GetREFDirectCommandQueue() const { return m_directCommandQueue; }
		const auto& GetREFDirectCommandList () const { return m_directCommandList; }

		auto& GetMutableREFSwapChain  () { return m_swapChain; }
		auto& GetMutableREFRenderGraph() { return m_renderGraph; }
		
		auto& GetMutableREFDirectCommandList() { return m_directCommandList; }

	private:

		void ResetCommandObjects(const FrameResource& a_frameResource);

		void DecideNextFrameUseFrameResource();

		bool PrepareForSwapChainResize();

		static constexpr std::size_t k_initialFrameResourceIndex   = 0ULL;
		static constexpr std::size_t k_frameResourceIndexIncrement = 1ULL;

		RootSignatureMap m_rootSignatureMap = {};
		PipelineStateMap m_pipelineStateMap = {};

		std::vector<std::shared_ptr<FrameResource>> m_frameResourceList = {};

		std::weak_ptr<FrameResource> m_currentFrameResource = {};

		SwapChain  m_swapChain  = {};
		RenderArea m_renderArea = {};

		DirectCommandQueue m_directCommandQueue = {};
		DirectCommandList  m_directCommandList  = {};

		RenderGraph m_renderGraph = {};

		Converter::RendererJsonConverter m_jsonConverter = {};

		std::size_t m_currentFrameResourceIndex = k_initialFrameResourceIndex;
	};
}