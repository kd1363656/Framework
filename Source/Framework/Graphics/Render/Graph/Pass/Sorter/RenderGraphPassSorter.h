#pragma once

namespace FWK::Graphics
{
	class RenderGraphPassSorter
	{
	public:

		 RenderGraphPassSorter() = default;
		~RenderGraphPassSorter() = default;

		void SortPassList(std::vector<std::unique_ptr<RenderGraphPassBase>>& a_passList) const;
		
	private:

		void AddPassExecutionLayerDependencyEdge(const std::vector<std::unique_ptr<RenderGraphPassBase>>& a_passList,
												 const std::size_t&									      a_beforePassIndex,
												 const std::size_t&									      a_afterPassIndex,
												       std::vector<std::vector<std::size_t>>&			  a_passDependencyList) const;

		void AddPassResourceDependencyEdge(const std::vector<std::unique_ptr<RenderGraphPassBase>>& a_passList,
									       const std::size_t&									    a_beforePassIndex,
									       const std::size_t&									    a_afterPassIndex,
									             std::vector<std::vector<std::size_t>>&				a_passDependencyList) const;

		void AddPassDependencyEdge(const std::size_t& a_beforePassIndex, const std::size_t& a_afterPassIndex, std::vector<std::vector<std::size_t>>& a_passDependencyList) const;

		bool IsSamePassExecutionLayer (const RenderGraphPassBase&			    a_lhs, const RenderGraphPassBase&			    a_rhs) const;
		bool IsSameRenderGraphResource(const Struct::RenderGraphResourceAccess& a_lhs, const Struct::RenderGraphResourceAccess& a_rhs) const;
		bool IsReadResourceAccess     (const Struct::RenderGraphResourceAccess& a_resourceAccess)						               const;
		bool IsWriteResourceAccess    (const Struct::RenderGraphResourceAccess& a_resourceAccess)						               const;

		static constexpr std::size_t k_minPassCountToResolveExecutionOrder = 2ULL;
		static constexpr std::size_t k_nextPassIndexOffset				   = 1ULL;
	};
}