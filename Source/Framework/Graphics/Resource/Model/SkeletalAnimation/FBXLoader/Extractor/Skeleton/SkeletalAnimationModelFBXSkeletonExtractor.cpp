#include "SkeletalAnimationModelFBXSkeletonExtractor.h"

bool FWK::Graphics::SkeletalAnimationModelFBXSkeletonExtractor::ExtractModelBoneList(const ufbx_scene* a_fbxScene, std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, std::vector<SkeletalAnimationModelRecord::ModelBone>& a_modelBoneList) const
{
	a_boneNodeIndexMap.clear();
	a_modelBoneList.clear   ();

	std::vector<const ufbx_node*> l_modelBoneNodeList = {};

	FWK_ASSERT_RETURN_VALUE_IF(!CollectModelBoneNodes(a_fbxScene, l_modelBoneNodeList),          "FBXシーンからBoneNodeの収集に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!CreateBoneNodeIndexMap(a_boneNodeIndexMap, l_modelBoneNodeList), "BoneNodeIndexMapの作成に失敗しました。",      false);

	a_modelBoneList.reserve(l_modelBoneNodeList.size());

	for (const auto* l_fbxBoneNode : l_modelBoneNodeList)
	{
		SkeletalAnimationModelRecord::ModelBone l_modelBone = {};

		FWK_ASSERT_RETURN_VALUE_IF(!CreateModelBone(a_boneNodeIndexMap, l_fbxBoneNode, l_modelBone), "ufbx_nodeからModelBoneへの変換に失敗しました", false);

		a_modelBoneList.emplace_back(std::move(l_modelBone));
	}

	FWK_ASSERT_RETURN_VALUE_IF(a_modelBoneList.empty(), "有効なModelBoneが存在しません。", false);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXSkeletonExtractor::CreateBoneNodeIndexMap(std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, const std::vector<const ufbx_node*>& a_modelBoneNodeList) const
{
	a_boneNodeIndexMap.clear  ();
	a_boneNodeIndexMap.reserve(a_modelBoneNodeList.size());

	for (auto l_boneNodeIndex = 0ULL; l_boneNodeIndex < a_modelBoneNodeList.size(); ++l_boneNodeIndex)
	{
		const auto* l_fbxBoneNode = a_modelBoneNodeList[l_boneNodeIndex];

		FWK_ASSERT_RETURN_VALUE_IF(!l_fbxBoneNode, "BoneNodeIndexMapへ登録するufbx_nodeがnullptrです。", false);

		const auto l_modelBoneIndex = static_cast<std::uint32_t>    (l_boneNodeIndex);
		const auto l_emplaceResult  = a_boneNodeIndexMap.try_emplace(l_fbxBoneNode, l_modelBoneIndex).second;

		FWK_ASSERT_RETURN_VALUE_IF(!l_emplaceResult, "同一BoneNodeがBoneNodeIndexMapへ重複登録されました。", false);
	}

	return true;
}
bool FWK::Graphics::SkeletalAnimationModelFBXSkeletonExtractor::CreateModelBone(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, const ufbx_node* a_fbxBoneNode, SkeletalAnimationModelRecord::ModelBone& a_modelBone) const
{
	a_modelBone = {};

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBoneNode, "ModelBoneへ変換するufbx_nodeがnullptrです。", false);

	// Bone名とBindPose時点の親相対行列を設定する
	a_modelBone.m_boneName            = Utility::ConvertUFBXStringToWString(a_fbxBoneNode->name);
	a_modelBone.m_bindPoseLocalMatrix = Utility::ConvertUFBXMatrixToMatrix (a_fbxBoneNode->node_to_parent);

	const auto* l_fbxParentBoneNode = a_fbxBoneNode->parent;

	// SceneRoot直下のBoneは親Boneを持たない
	if (!l_fbxParentBoneNode ||
	     l_fbxParentBoneNode->is_root)
	{
		a_modelBone.m_parentBoneIndex = SkeletalAnimationModelRecord::k_invalidBoneIndex;

		return true;
	}

	const auto& l_parentBoneIndexITR = a_boneNodeIndexMap.find(l_fbxParentBoneNode);

	FWK_ASSERT_RETURN_VALUE_IF(l_parentBoneIndexITR == a_boneNodeIndexMap.end(), "ModelBoneの親NodeがBoneNodeIndexMapに存在しません。", false);

	a_modelBone.m_parentBoneIndex = l_parentBoneIndexITR->second;

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXSkeletonExtractor::CollectModelBoneNodes(const ufbx_scene* a_fbxScene, std::vector<const ufbx_node*>& a_modelBoneNodeList) const
{
	a_modelBoneNodeList.clear();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,            "BoneNodeを収集するufbx_sceneがnullptrです。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene->root_node, "ufbx_sceneにRootNodeが存在しません。",        false);

	std::unordered_set<const ufbx_node*> l_registeredBoneNodeSet = {};

	for (auto l_nodeIndex = 0ULL; l_nodeIndex < a_fbxScene->nodes.count; ++l_nodeIndex)
	{
		const auto* l_fbxNode = a_fbxScene->nodes.data[l_nodeIndex];

		if (!l_fbxNode)       { continue; }
		if (!l_fbxNode->mesh) { continue; }

		const auto* l_fbxMesh = l_fbxNode->mesh;

		// SkinDeformerを持たないMeshは対象外
		if (l_fbxMesh->skin_deformers.count == Constant::k_emptyUFBXElementCount) { continue; }

		FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->skin_deformers.count != Constant::k_supportedSkinDeformerCount, "一つのufbx_meshに設定されたSkinDeformer数が1個ではありません。", false);

		const auto* l_fbxSkinDeformer = l_fbxMesh->skin_deformers.data[Constant::k_initialSkinDeformerIndex];

		FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinDeformer,                        "ufbx_skin_deformerがnullptrです。",         false);
		FWK_ASSERT_RETURN_VALUE_IF(l_fbxSkinDeformer->clusters.count == 0ULL, "SkinDeformerにSkinClusterが存在しません。", false);

		for (auto l_clusterIndex = 0ULL; l_clusterIndex < l_fbxSkinDeformer->clusters.count; ++l_clusterIndex)
		{
			const auto* l_fbxSkinCluster = l_fbxSkinDeformer->clusters.data[l_clusterIndex];

			FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinCluster,            "ufbx_skin_clusterがnullptrです。",     false);
			FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinCluster->bone_node, "SkinClusterのBoneNodeがnullptrです。", false);

			std::vector<const ufbx_node*> l_childToParentBoneNodeList = {};

			// SkinClusterのBoneからSceneRoot直下までの親を収集する
			auto* l_fbxBoneNode = l_fbxSkinCluster->bone_node;

			while (l_fbxBoneNode)
			{
				if (l_fbxBoneNode->is_root ||
					l_registeredBoneNodeSet.contains(l_fbxBoneNode))
				{
					break;
				}

				l_childToParentBoneNodeList.emplace_back(l_fbxBoneNode);

				l_fbxBoneNode = l_fbxBoneNode->parent;
			}

			// 子から親の順で収集したため、順番を並び変える
			std::reverse(l_childToParentBoneNodeList.begin(), l_childToParentBoneNodeList.end());

			for (const auto* l_hierarchyBoneNode : l_childToParentBoneNodeList)
			{
				if (!l_registeredBoneNodeSet.emplace(l_hierarchyBoneNode).second) { continue; }

				a_modelBoneNodeList.emplace_back(l_hierarchyBoneNode);
			}
		}
	}

	FWK_ASSERT_RETURN_VALUE_IF(a_modelBoneNodeList.empty(), "SkinClusterから有効なBoneNodeを収集できませんでした。", false);

	return true;

}