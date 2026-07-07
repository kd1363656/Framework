#pragma once

namespace FWK::Graphics
{
	class Renderer final
	{
	private:

		using RootSignatureMap = std::unordered_map<Enum::RootSignatureType, std::shared_ptr<RootSignature>>;
		using PipelineStateMap = std::unordered_map<Enum::PipelineStateType, std::shared_ptr<PipelineStateBase>>;
		
	public:

		 Renderer() = default;
		~Renderer() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool PostDeserialize(const Device&			   a_device, 
							 const Window&			   a_window,
							 const Factory&			   a_factory,
							 const Struct::ClientSize& a_clientSize,
								   ResourceContext&    a_resourceContext);
		
		void BeginFrame(const ResourceContext& a_resourceContext);
		void Execute   (const ResourceContext& a_resourceContext);
		void EndFrame  ();

		nlohmann::json Serialize() const;

		void Resize(const Device& a_device, const Struct::ClientSize& a_clientSize, ResourceContext& a_resourceContext);

		void AddFrameResource(const std::shared_ptr<FrameResource>&     a_frameResource);
		void AddRootSignature(const std::shared_ptr<RootSignature>&     a_rootSignature, const Enum::RootSignatureType a_rootSignatureType);
		void AddPipelineState(const std::shared_ptr<PipelineStateBase>& a_pipelineState, const Enum::PipelineStateType a_pipelineStateType);

		std::weak_ptr<RootSignature> FindVALRootSignature(const Enum::RootSignatureType a_rootSignatureType) const;

		template <Concept::IsDerivedPipelineStateBaseConcept PipelineStateType>
		std::weak_ptr<PipelineStateBase> FindVALPipelineState(const Enum::PipelineStateType a_pipelineStateType) const
		{
			const auto& l_itr = m_pipelineStateMap.find(a_pipelineStateType);

			if (l_itr == m_pipelineStateMap.end()) { return {}; }

			auto& l_pipelineState = l_itr->second;

			if (!l_pipelineState) { return {}; }

			// もし基底クラスの型情報と、派生クラスの型情報が一致したらキャスト
			// 一致しなければreturn
			if (!Utility::IsDerivedFrom(l_pipelineState->GetREFRuntimeTypeINFO(), PipelineStateType::GetREFTypeINFO())) { return {}; }

			return std::static_pointer_cast<PipelineStateType>(l_pipelineState);
		}

		const auto& GetREFFrameResourceList() const { return m_frameResourceList; }
		const auto& GetREFRootSignatureMap () const { return m_rootSignatureMap; }
		const auto& GetREFPipelineStateMap () const { return m_pipelineStateMap; }

		const auto& GetREFCurrentFrameResource() const { return m_currentFrameResource; }

		const auto& GetREFSwapChain () const { return m_swapChain; }
		const auto& GetREFRenderArea() const { return m_renderArea; }

		const auto& GetREFRenderGraph() const { return m_renderGraph; }
		
		const auto& GetREFDirectCommandQueue() const { return m_directCommandQueue; }
		const auto& GetREFDirectCommandList () const { return m_directCommandList; }

		const auto& GetREFCurrentFrameResourceIndex() const { return m_currentFrameResourceIndex; }

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

		DirectCommandQueue  m_directCommandQueue  = {};
		DirectCommandList   m_directCommandList   = {};
		ComputeCommandQueue m_computeCommandQueue = {};
		ComputeCommandList  m_computeCommandList  = {};

		RenderGraph m_renderGraph = {};

		Converter::RendererJsonConverter m_jsonConverter = {};

		std::size_t m_currentFrameResourceIndex = k_initialFrameResourceIndex;
	};
}