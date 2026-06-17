#pragma once

namespace FWK::Graphics
{
	class StaticModelMeshletBuilder final
	{
	public:

		StaticModelMeshletBuilder () = default;
		~StaticModelMeshletBuilder() = default;

		bool BuildStaticModelRecordMeshletData(Graphics::StaticModelRecord& a_staticModelRecord) const;

	private:

		bool BuildModelMeshletData(Struct::StaticModelMesh& a_staticModelMesh) const;

		static constexpr std::size_t k_vectorElementIndexX = 0ULL;
		static constexpr std::size_t k_vectorElementIndexY = 1ULL;
		static constexpr std::size_t k_vectorElementIndexZ = 2ULL;

		static constexpr std::uint32_t k_emptyRemainder	= 0U;
	};
}