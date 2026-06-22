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
												 std::vector<std::vector<std::size_t>>&					  a_passDependencyList,
												 std::vector<std::size_t>);

	};
}