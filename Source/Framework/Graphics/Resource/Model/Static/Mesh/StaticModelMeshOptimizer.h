#pragma once

namespace FWK::Graphics
{
	class StaticModelMeshOptimizer final
	{
	public:

		 StaticModelMeshOptimizer() = default;
		~StaticModelMeshOptimizer() = default;

		bool OptimizeStaticModelRecord(Graphics::StaticModelRecord& a_staticModelRecord) const;

	private:

		bool OptimizeStaticModelMesh(StaticModelRecord::StaticModelMesh& a_staticModelMesh) const;

		static constexpr std::size_t k_invalidOptimizedVertexCount = 0ULL;
	};
}