#include "SkeletalAnimationModelFBXLoader.h"

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::LoadSkeletalAnimationModelFile(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const
{
	auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

	// 以前読み込んだデータと今回のデータが混在しないように
	// FBXを読み込む前にAssetDataを空にする
	l_modelData.m_modelMeshList.clear     ();
	l_modelData.m_boneList.clear          ();
	l_modelData.m_motionSequenceList.clear();

	// FBXファイル全体をufbx_sceneとして読み込む
	auto* l_fbxScene = LoadFBXScene(a_filePath);

	FWK_ASSERT_RETURN_VALUE_IF(!l_fbxScene, "FBXシーンの読み込みに失敗したため、Modelファイルの読み込みに失敗しました。", false);

	// SkeletonとMeshをModelDataへ変換する
	if (!ExtractModelData(l_fbxScene, l_modelData))
	{
		DestroyFBXScene(l_fbxScene);

		l_modelData.m_modelMeshList.clear     ();
		l_modelData.m_boneList.clear          ();
		l_modelData.m_motionSequenceList.clear();

		FWK_ASSERT_RETURN_VALUE("FBXシーンからModelDataの抽出に失敗しました。", false);
	}

	// ModelDataへ必要な情報をコピーし終えたため破棄する
	DestroyFBXScene(l_fbxScene);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CreateBoneNodeIndexMap(const std::vector<const ufbx_node*>& a_modelBoneNodeList, BoneNodeIndexMap& a_boneNodeIndexMap) const
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

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CreateModelBone(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_node* a_fbxBoneNode, SkeletalAnimationModelRecord::ModelBone& a_modelBone) const
{
	a_modelBone = {};

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBoneNode, "ModelBoneへ変換するufbx_nodeがnullptrです。", false);

	// Bone名とBindPose時点の親相対行列を設定する
	a_modelBone.m_boneName = ConvertUFBXStringToWString(a_fbxBoneNode->name);
	a_modelBone.m_bindPoseLocalMatrix = ConvertUFBXMatrixToMatrix(a_fbxBoneNode->node_to_parent);

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

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ApplyModelVertexBoneInfluence(const BoneNodeIndexMap&                                                   a_boneNodeIndexMap, 
	                                                                               const ufbx_mesh*                                                          a_fbxMesh, 
	                                                                               const ufbx_skin_deformer*                                                 a_fbxSkinDeformer, 
	                                                                               const std::uint32_t                                                       a_fbxVertexIndex, 
	                                                                                     BoneIndexPaletteIndexMap&                                           a_boneIndexPaletteIndexMap,
	                                                                                     std::vector<SkeletalAnimationModelRecord::ModelBonePaletteElement>& a_bonePaletteList, 
	                                                                                     SkeletalAnimationModelRecord::ModelVertex&                          a_modelVertex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh,                                          "BoneInfluenceを取得するufbx_meshがnullptrです。",          false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxSkinDeformer,                                  "BoneInfluenceを取得するufbx_skin_deformerがnullptrです。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxVertexIndex >= a_fbxMesh->vertex_indices.count, "BoneInfluenceを取得するVertexIndexが範囲外です。",         false);

	// 三角形化後のIndexからSkinWeight用の論理VertexIndexを取得する
	const auto l_logicalVertexIndex = a_fbxMesh->vertex_indices.data[a_fbxVertexIndex];

	FWK_ASSERT_RETURN_VALUE_IF(l_logicalVertexIndex >= a_fbxSkinDeformer->vertices.count, "SkinWeight用の論理VertexIndexが範囲外です。", false);

	const auto& l_fbxSkinVertex = a_fbxSkinDeformer->vertices.data[l_logicalVertexIndex];

	const auto& l_skinWeightEnd = static_cast<std::size_t>(l_fbxSkinVertex.weight_begin + l_fbxSkinVertex.num_weights);

	FWK_ASSERT_RETURN_VALUE_IF(l_skinWeightEnd > a_fbxSkinDeformer->weights.count, "SkinWeight配列の参照範囲が不正です。", false);

	a_modelVertex.m_bonePaletteIndex0 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndex1 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndex2 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndex3 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_boneWeight        = {};

	std::uint32_t l_appliedBoneInfluenceCount = k_emptyBoneInfluenceCount;

	for (auto l_skinWeightOffset = 0U; l_skinWeightOffset < l_fbxSkinVertex.num_weights; ++l_skinWeightOffset)
	{
		const auto  l_skinWeightIndex = l_fbxSkinVertex.weight_begin + l_skinWeightOffset;
		const auto& l_fbxSkinWeight   = a_fbxSkinDeformer->weights.data[l_skinWeightIndex];

		// Weightは降順なので0以下になった時点で終了する
		if (l_appliedBoneInfluenceCount < k_maxBoneInfluenceCount ||
			l_fbxSkinWeight.weight <= k_emptyBoneWeight)
		{
			break; 
		}

		FWK_ASSERT_RETURN_VALUE_IF(l_fbxSkinWeight.cluster_index >= a_fbxSkinDeformer->clusters.count, "SkinWeightが参照するClusterIndexが範囲外です。", false);

		const auto* l_fbxSkinCluster = a_fbxSkinDeformer->clusters.data[l_fbxSkinWeight.cluster_index];

		FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinCluster,            "SkinWeightが参照するufbx_skin_clusterがnullptrです。", false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinCluster->bone_node, "SkinClusterが参照するBoneNodeがnullptrです。",         false);

		const auto& l_boneNodeIndexITR = a_boneNodeIndexMap.find(l_fbxSkinCluster->bone_node);

		FWK_ASSERT_RETURN_VALUE_IF(l_boneNodeIndexITR == a_boneNodeIndexMap.end(), "SkinClusterが参照するBoneNodeがBoneNodeIndexMapに存在しません。", false);

		const auto l_modelBoneIndex = l_boneNodeIndexITR->second;

		std::uint32_t l_bonePaletteIndex = SkeletalAnimationModelRecord::k_invalidPaletteIndex;

		// 既にPaletteへ登録しているBoneなら既存Indexを使用する
		if (const auto& l_bonePaletteIndexITR = a_boneIndexPaletteIndexMap.find(l_modelBoneIndex);
			l_bonePaletteIndexITR != a_boneIndexPaletteIndexMap.end())
		{
			l_bonePaletteIndex = l_bonePaletteIndexITR->second;
		}
		else
		{
			// このMeshで初めて使用するBoneをPaletteへ登録する
			l_bonePaletteIndex = static_cast<std::uint32_t>(a_bonePaletteList.size());

			SkeletalAnimationModelRecord::ModelBonePaletteElement l_bonePaletteElement = {};

			l_bonePaletteElement.m_inverseBindPoseMatrix = ConvertUFBXMatrixToMatrix(l_fbxSkinCluster->geometry_to_bone);
			l_bonePaletteElement.m_boneIndex             = l_modelBoneIndex;

			a_bonePaletteList.emplace_back        (l_bonePaletteElement);
			a_boneIndexPaletteIndexMap.try_emplace(l_modelBoneIndex, l_bonePaletteIndex);
		}

		ApplyModelVertexBoneInfluenceSlot(static_cast<float>(l_fbxSkinWeight.weight),  
				                                             l_appliedBoneInfluenceCount,
				                                             l_bonePaletteIndex,
				                                             a_modelVertex);
			
		++l_appliedBoneInfluenceCount;
	}

	FWK_ASSERT_RETURN_VALUE_IF(l_appliedBoneInfluenceCount == k_emptyBoneInfluenceCount, "ModelVertexへ適用できるBoneInfluenceが存在しません。", false);

	// Weightが0の未使用Slotにも有効なPaletteIndexを設定する
	const auto l_fallbackBonePaletteIndex = a_modelVertex.m_bonePaletteIndex0;

	for (auto l_slotIndex = l_appliedBoneInfluenceCount; l_slotIndex < k_maxBoneInfluenceCount; ++l_slotIndex)
	{
		ApplyModelVertexBoneInfluenceSlot(k_emptyBoneWeight,
										  l_slotIndex,
										  l_fallbackBonePaletteIndex,
										  a_modelVertex);
	}

	return NormalizeModelVertexBoneWeight(a_modelVertex);

}
void FWK::Graphics::SkeletalAnimationModelFBXLoader::ApplyModelVertexBoneInfluenceSlot(const float                                      a_boneWeight, 
	                                                                                   const std::uint32_t                              a_slotIndex, 
	                                                                                   const std::uint32_t                              a_bonePaletteIndex, 
	                                                                                         SkeletalAnimationModelRecord::ModelVertex& a_modelVertex) const
{
	switch(a_slotIndex)
	{
		case k_firstBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndex0 = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.x      = a_boneWeight;
		}
		break;

		case k_secondBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndex1 = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.y      = a_boneWeight;
		}
		break;

		case k_thirdBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndex2 = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.x      = a_boneWeight;
		}
		break;

		case k_fourthBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndex3 = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.w      = a_boneWeight;
		}
		break;

		default:
		{
			FWK_ASSERT_RETURN("BoneInfluenceを設定するSlotIndexが範囲外です。");
		}
		break;
	}
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelData(const ufbx_scene* a_fbxScene, SkeletalAnimationModelRecord::ModelData& a_modelData) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,                                                "ufbx_sceneが無効のため、SkeletalAnimationModelDataの抽出に失敗しました。",            false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxScene->nodes.count == Constant::k_emptyModelMeshCount, "FBXシーン内にNodeが存在しないため、SkeletalAnimationModelDataの抽出に失敗しました。", false);


	return false;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMeshList(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_node* a_fbxNode, const std::size_t& a_materialIndex, SkeletalAnimationModelRecord::ModelMesh& a_modelMesh)
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelBoneList(const ufbx_scene* a_fbxScene, BoneNodeIndexMap& a_boneNodeIndexMap, std::vector<SkeletalAnimationModelRecord::ModelBone>& a_modelBoneList) const
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMeshByMaterial(const std::size_t&                             a_materialIndex, 
																				const ufbx_node*                               a_fbxNode, 
																			    const BoneNodeIndexMap&                        a_boneNodeIndexMap,		
																					  SkeletalAnimationModelRecord::ModelMesh& a_modelMes)
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CollectModelBoneNodes(const ufbx_scene* a_fbxScene, std::vector<const ufbx_node*>& a_modelBoneNodexList) const
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::NormalizeModelVertexBoneWeight(SkeletalAnimationModelRecord::ModelVertex& a_modelVertex) const
{

}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexPosition(const ufbx_mesh * a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	return TypeAlias::Math::Vector3();
}
FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexNormal(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	return TypeAlias::Math::Vector3();
}
FWK::TypeAlias::Math::Vector4 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexTangent(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	return TypeAlias::Math::Vector4();
}

FWK::TypeAlias::Math::Matrix FWK::Graphics::SkeletalAnimationModelFBXLoader::ConvertUFBXMatrixToMatrix(const ufbx_matrix& a_fbxMatrix) const
{
	return TypeAlias::Math::Matrix();
}
FWK::TypeAlias::Math::Matrix FWK::Graphics::SkeletalAnimationModelFBXLoader::ConvertUFBXTransformToMatrix(const ufbx_transform& a_fbxTransform) const
{
	return TypeAlias::Math::Matrix();
}
FWK::TypeAlias::Math::Quaternion FWK::Graphics::SkeletalAnimationModelFBXLoader::ConvertUFBXQuaternionToQuaternion(const ufbx_quat& a_fbxQuaternion) const
{
	return TypeAlias::Math::Quaternion();
}