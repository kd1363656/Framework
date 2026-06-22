#pragma once

namespace FWK::Utility
{
	class TopologicalSorter final
	{
	public:

		 TopologicalSorter() = default;
		~TopologicalSorter() = default;

		std::vector<std::size_t> Sort(const std::vector<std::vector<std::size_t>>& a_dependencyList) const;

	private:

		static constexpr std::size_t k_emptyInDegree = 0ULL;
	};
}