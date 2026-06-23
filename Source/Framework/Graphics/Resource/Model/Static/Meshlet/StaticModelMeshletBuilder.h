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

		bool PackPrimitiveIndexList(const std::vector<std::uint8_t>& a_sourcePrimitiveIndexList, const std::size_t& a_usedPrimitiveIndexCount, std::vector<std::uint32_t>& a_packedPrimitiveIndexList) const;

		static constexpr float k_defaultMeshletConeWeight    = 0.25F;
		static constexpr float k_defaultMeshletBoundsPadding = 0.0F;

		static constexpr std::size_t k_vectorElementIndexX = 0ULL;
		static constexpr std::size_t k_vectorElementIndexY = 1ULL;
		static constexpr std::size_t k_vectorElementIndexZ = 2ULL;

		static constexpr std::size_t k_meshletPrimitiveIndexAlignment = 4ULL;

		static constexpr std::size_t k_packedPrimitiveIndexByteCount = 4ULL;

		static constexpr std::uint32_t k_packedPrimitiveIndexBitCount = 8U;

		static constexpr std::uint32_t k_emptyRemainder	= 0U;
	};
}