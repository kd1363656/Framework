#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelMeshOptimizer final : public ModelMeshOptimizerBase
	{
	public:

		 SkeletalAnimationModelMeshOptimizer()          = default;
		~SkeletalAnimationModelMeshOptimizer() override = default;

		bool OptimizeSkeletalAnimationModelRecord(Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const;

	private:

		bool OptimizeSkeletalAnimationModelMesh(SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const;

		static constexpr std::size_t k_invalidOptimizeVertexCount = 0ULL;
	};
}