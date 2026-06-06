#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	class RenderGraph
	{
	private:

		struct ResourceAccessPassRecord final
		{
			Enum::RenderGraphResourceType m_resourceType = Enum::RenderGraphResourceType::None;

			std::uint32_t m_passIndex   = 0U;
			std::uint32_t m_accessOrder = 0U;

			bool m_isRead             = false;
			bool m_isWrite            = false;
			bool m_isFinalStateAccess = false;
		};

		struct ResourceStateRecord final
		{
			Enum::RenderGraphResourceType m_resourceType = Enum::RenderGraphResourceType::None;

			D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
		};

	public:

		 RenderGraph() = default;
		~RenderGraph() = default;

		void Deserialize				(const nlohmann::json& a_rootJson);
		void BuildDefaultBackBufferGraph();
		bool Compile					();
		
		void BeginFrame();

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer);

		void AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass);

		nlohmann::json Serialize() const;

		const auto& GetREFPassList() const { return m_passList; }

	private:

		bool IsReadAccess	   (const Enum::RenderGraphResourceAccessType a_accessType) const;
		bool IsWriteAccess	   (const Enum::RenderGraphResourceAccessType a_accessType) const;
		bool IsFinalStateAccess(const Enum::RenderGraphResourceAccessType a_accessType) const;

		void AddDependencyEdge(const std::uint32_t							  a_fromPassIndex, 
							   const std::uint32_t							  a_toPassIndex,
									 std::vector<std::vector<std::uint32_t>>& a_edgeList,
									 std::vector<std::uint32_t>&			  a_inDegreeList) const;

		void BuildDependency(std::vector<std::vector<std::uint32_t>>& a_edgeList, std::vector<std::uint32_t>& a_inDegreeList) const;

		void TransitionPassResources	 (const RenderGraphPassBase&   a_pass,		 Renderer& a_renderer);
		void TransitionBackBufferResource(const D3D12_RESOURCE_STATES& a_afterState, Renderer& a_renderer);

		D3D12_RESOURCE_STATES ConvertAccessTypeToResourceState(const Enum::RenderGraphResourceAccessType a_accessType) const;

		void SetupCurrentResourceState(const Enum::RenderGraphResourceType a_resourceType, const D3D12_RESOURCE_STATES a_currentState);

		D3D12_RESOURCE_STATES FetchVALCurrentResourceState(const Enum::RenderGraphResourceType a_resourceType) const;

		static constexpr std::uint32_t k_noIncomingEdgeCount = 0U;
		static constexpr std::uint32_t k_invalidPassIndex    = std::numeric_limits<std::uint32_t>::max();

		std::vector<std::unique_ptr<RenderGraphPassBase>> m_passList			    = {};
		std::vector<std::uint32_t>						  m_sortedPassIndexList     = {};
		std::vector<ResourceStateRecord>				  m_resourceStateRecordList = {};

		Converter::RenderGraphJsonConverter m_jsonConverter = {};
	};
}