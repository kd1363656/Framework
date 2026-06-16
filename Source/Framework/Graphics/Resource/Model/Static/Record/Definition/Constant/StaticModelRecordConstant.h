#pragma once

namespace FWK::Constant
{
	inline constexpr TypeAlias::Math::Color k_defaultModelMaterialBaseColorFactor =
	{
		1.0F,
		1.0F,
		1.0F,
		1.0F
	};

	inline constexpr float k_defaultModelMaterialRoughnessFactor = 1.0F;
	inline constexpr float k_defaultModelMaterialMetallicFactor  = 0.0F;

	inline constexpr float k_initialMeshletBoundsRadius     = 0.0F;
	inline constexpr float k_initialMeshletBoundsConeCutoff = 0.0F;
	inline constexpr float k_initialMeshletBoundsPadding    = 0.0F;

	inline constexpr std::uint32_t k_initialMeshletVertexOffset   = 0U;
	inline constexpr std::uint32_t k_initialMeshletTriangleOffset = 0U;
	inline constexpr std::uint32_t k_initialMeshletVertexCount    = 0U;
	inline constexpr std::uint32_t k_initialMeshletTriangleCount  = 0U;
}