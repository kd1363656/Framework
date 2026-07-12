#include "SkeletalAnimationModelMeshOptimizer.h"

bool FWK::Graphics::SkeletalAnimationModelMeshOptimizer::OptimizeSkeletalAnimationModelRecord(Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const
{
	auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

	FWK_ASSERT_RETURN_VALUE_IF(l_modelData.m_modelMeshList.empty(), "ModelDataのMeshリストが空のため、ModelMeshの最適化に失敗しました。", false);

	for (auto& l_modelMesh : l_modelData.m_modelMeshList)
	{
		// SkeletalAnimationModelDataはMaterial単位などで複数のModelMeshを持つ。
		// meshoptimizerはMesh単位のVertexListとIndexListに対して適用するため、
		// ModelMeshごとに個別に最適化する

		FWK_ASSERT_RETURN_VALUE_IF(!OptimizeSkeletalAnimationModelMesh(l_modelMesh), "SkeletalAnimationModelMeshの最適化に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelMeshOptimizer::OptimizeSkeletalAnimationModelMesh(SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const
{
	return false;
}