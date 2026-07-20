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

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelData(const ufbx_scene* a_fbxScene, SkeletalAnimationModelRecord::ModelData& a_modelData) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,                                                  "ufbx_sceneが無効のため、ModelDataの抽出に失敗しました。",            false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxScene->nodes.count == Constant::k_emptyUFBXElementCount, "FBXシーン内にNodeが存在しないため、ModelDataの抽出に失敗しました。", false);

	a_modelData.m_modelMeshList.clear     ();
	a_modelData.m_boneList.clear          ();
	a_modelData.m_motionSequenceList.clear();

	std::unordered_map<const ufbx_node*, std::uint32_t> l_boneNodeIndexMap = {};

	// Meshより先にBoneIndexを確定する
	FWK_ASSERT_RETURN_VALUE_IF(!m_skeletonExtractor.ExtractModelBoneList(a_fbxScene, l_boneNodeIndexMap, a_modelData.m_boneList), "FBXシーンからModelBoneListの抽出に失敗しました。", false);

	for (auto l_nodeIndex = 0ULL; l_nodeIndex < a_fbxScene->nodes.count; ++l_nodeIndex)
	{
		const auto* l_fbxNode = a_fbxScene->nodes.data[l_nodeIndex];

		if (!l_fbxNode) { continue; }

		const auto& l_fbxMesh = l_fbxNode->mesh;

		if (!l_fbxMesh) { continue; }

		// SkinDeformerを持たないMeshは対象外
		if (l_fbxMesh->skin_deformers.count == Constant::k_emptyUFBXElementCount) { continue; }

		std::vector<SkeletalAnimationModelRecord::ModelMesh> l_modelMeshList = {};

		FWK_ASSERT_RETURN_VALUE_IF(!ExtractModelMeshList(l_boneNodeIndexMap, l_fbxNode, l_modelMeshList), "ufbx_nodeからModelMeshListの抽出に失敗しました。", false);

		for (auto& l_modelMesh : l_modelMeshList)
		{
			if (l_modelMesh.m_modelVertexList.empty()) { continue; }
			if (l_modelMesh.m_indexList.empty())       { continue; }
			if (l_modelMesh.m_bonePaletteList.empty()) { continue; }

			a_modelData.m_modelMeshList.emplace_back(std::move(l_modelMesh));
		}
	}

	FWK_ASSERT_RETURN_VALUE_IF(a_modelData.m_modelMeshList.empty(), "有効なSkeletalAnimationModelMeshが存在しません。", false);

	// FBX内のAnimationStackをMotionSequenceへ変換する
    FWK_ASSERT_RETURN_VALUE_IF(!m_motionExtractor.ExtractModelMotionSequenceList(
    							l_boneNodeIndexMap,
    							a_fbxScene,
    							a_modelData.m_motionSequenceList),
    							"FBXシーンからMotionSequenceListの抽出に失敗しました。",
    							false);
	return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMeshList(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, const ufbx_node* a_fbxNode, std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const
{
	a_modelMeshList.clear();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode,       "ufbx_nodeがnullptrのため、ModelMeshListの抽出に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode->mesh, "ufbx_nodeにMeshが存在しないため、ModelMeshListの抽出に失敗しました。", false);

	const auto* l_fbxMesh = a_fbxNode->mesh;

	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->skin_deformers.count != Constant::k_supportedSkinDeformerCount, "一つのufbx_meshに設定されたSkinDeformer数が1個ではありません。", false);

	const auto* l_fbxSkinDeformer = l_fbxMesh->skin_deformers.data[Constant::k_initialSkinDeformerIndex];

	FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinDeformer, "ufbx_skin_deformerがnullptrです。", false);

	const bool l_isSupportedSkinningMethod = l_fbxSkinDeformer->skinning_method == UFBX_SKINNING_METHOD_LINEAR || 
											 l_fbxSkinDeformer->skinning_method == UFBX_SKINNING_METHOD_RIGID; 

	FWK_ASSERT_RETURN_VALUE_IF(!l_isSupportedSkinningMethod, "LinearまたはRigid以外のSkinningMethodには対応していません。", false);

	// Materialがない場合は全Faceを一つのModelMeshへ変換する
	if (l_fbxMesh->materials.count == Constant::k_emptyUFBXElementCount)
	{
		SkeletalAnimationModelRecord::ModelMesh l_modelMesh = {};

		FWK_ASSERT_RETURN_VALUE_IF(!ExtractModelMeshByMaterial(k_invalidMaterialIndex,
															   a_fbxNode,
			                                                   a_boneNodeIndexMap,
			                                                   l_modelMesh), 
			                                                   "MaterialなしModelMeshの抽出に失敗しました。", 
			                                                   false);

		if (!l_modelMesh.m_modelVertexList.empty() &&
			!l_modelMesh.m_indexList.empty())
		{
			l_modelMesh.m_modelMaterial = {};

			a_modelMeshList.emplace_back(std::move(l_modelMesh));
		}

		return true;
	}

	for (std::size_t l_materialIndex = 0ULL; l_materialIndex < l_fbxMesh->materials.count; ++l_materialIndex)
	{
		SkeletalAnimationModelRecord::ModelMesh l_modelMesh = {};

		FWK_ASSERT_RETURN_VALUE_IF(!ExtractModelMeshByMaterial(l_materialIndex, a_fbxNode, a_boneNodeIndexMap, l_modelMesh), "Material別ModelMeshの抽出に失敗しました。", false);

		if (l_modelMesh.m_modelVertexList.empty()) { continue; }
		if (l_modelMesh.m_indexList.empty())       { continue; }

		const auto* l_fbxMaterial = l_fbxMesh->materials.data[l_materialIndex];

		ExtractModelMaterial(l_fbxMaterial, l_modelMesh.m_modelMaterial.m_modelMaterialAssetData);
		
		l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData = {};

		a_modelMeshList.emplace_back(std::move(l_modelMesh));
	}

	return true;

}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ApplyModelVertexBoneInfluence(const std::unordered_map<const ufbx_node*, std::uint32_t>&                a_boneNodeIndexMap, 
	                                                                               const ufbx_mesh*                                                          a_fbxMesh,
	                                                                               const ufbx_skin_deformer*                                                 a_fbxSkinDeformer,
	                                                                               const std::uint32_t                                                       a_fbxVertexIndex, 
	                                                                                     std::unordered_map<std::uint32_t, std::uint32_t>&                   a_boneIndexPaletteIndexMap, 
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

	// 加算によるOverflowを避けるため、まず開始Indexだけを確認する
	FWK_ASSERT_RETURN_VALUE_IF(l_fbxSkinVertex.weight_begin > a_fbxSkinDeformer->weights.count, "SkinWeightの開始Indexが範囲外です。", false);

	const auto l_remainingSkinWeightCount = a_fbxSkinDeformer->weights.count - l_fbxSkinVertex.weight_begin;

	FWK_ASSERT_RETURN_VALUE_IF(l_fbxSkinVertex.num_weights > l_remainingSkinWeightCount, "SkinWeight配列の参照範囲が不正です。", false);
	
	a_modelVertex.m_boneWeight            = {};
	a_modelVertex.m_bonePaletteIndexZero  = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndexOne   = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndexTwo   = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndexThree = SkeletalAnimationModelRecord::k_invalidPaletteIndex;

	std::uint32_t l_appliedBoneInfluenceCount = k_emptyBoneInfluenceCount;

	for (auto l_skinWeightOffset = 0U; l_skinWeightOffset < l_fbxSkinVertex.num_weights; ++l_skinWeightOffset)
	{
		// ModelVertexへ格納するInfluenceはWeightが大きい先頭4件まで
		if (l_appliedBoneInfluenceCount >= k_maxBoneInfluenceCount) { break; }

		const auto  l_skinWeightIndex = l_fbxSkinVertex.weight_begin + l_skinWeightOffset;
		const auto& l_fbxSkinWeight = a_fbxSkinDeformer->weights.data[l_skinWeightIndex];

		// Weightは降順なので0以下になった時点で終了する
		if (l_fbxSkinWeight.weight <= k_emptyBoneWeight) { break; }

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

			l_bonePaletteElement.m_inverseBindPoseMatrix = Utility::ConvertUFBXMatrixToMatrix(l_fbxSkinCluster->geometry_to_bone);
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
	const auto l_fallbackBonePaletteIndex = a_modelVertex.m_bonePaletteIndexZero;

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
			a_modelVertex.m_bonePaletteIndexZero = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.x         = a_boneWeight;
		}
		break;

		case k_secondBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndexOne = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.y        = a_boneWeight;
		}
		break;

		case k_thirdBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndexTwo = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.z        = a_boneWeight;
		}
		break;

		case k_fourthBoneInfluenceSlot:
		{
			a_modelVertex.m_bonePaletteIndexThree = a_bonePaletteIndex;
			a_modelVertex.m_boneWeight.w          = a_boneWeight;
		}
		break;

		default:
		{
			FWK_ASSERT_RETURN("BoneInfluenceを設定するSlotIndexが範囲外です。");
		}
		break;
	}
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMeshByMaterial(const std::size_t&                                         a_materialIndex, 
	                                                                            const ufbx_node*                                           a_fbxNode,
	                                                                            const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, 
	                                                                                  SkeletalAnimationModelRecord::ModelMesh&             a_modelMesh) const
{
	a_modelMesh.m_modelVertexList.clear();
	a_modelMesh.m_bonePaletteList.clear();
	a_modelMesh.m_indexList.clear      ();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode,       "ufbx_nodeがnullptrのため、Material別ModelMeshの抽出に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode->mesh, "ufbx_nodeにMeshが存在しないため、Material別ModelMeshの抽出に失敗しました。", false);

	const auto* l_fbxMesh = a_fbxNode->mesh;

	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->skin_deformers.count != Constant::k_supportedSkinDeformerCount,  "一つのufbx_meshに設定されたSkinDeformer数が1個ではありません。", false);
	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->faces.count == Constant::k_emptyUFBXElementCount,                "三角形化できるFaceが存在しません。",                             false);
	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->max_face_triangles == Constant::k_emptyModelMeshCount,           "三角形化できるFaceが存在しません。",                             false);

	FWK_ASSERT_RETURN_VALUE_IF(a_materialIndex != k_invalidMaterialIndex &&
							   l_fbxMesh->face_material.count != l_fbxMesh->faces.count,
							   "face_material数とFace数が一致しません。",
							   false);

	FWK_ASSERT_RETURN_VALUE_IF(a_materialIndex != k_invalidMaterialIndex &&
						       a_materialIndex >= l_fbxMesh->materials.count, 
						       "MateirlaIndexが範囲外です。",
						       false);

	const auto* l_fbxSkinDeformer = l_fbxMesh->skin_deformers.data[Constant::k_initialSkinDeformerIndex];

	FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinDeformer, "ufbx_skin_deformerがnullptrです。", false);

	const auto& l_triangleIndexListSize = l_fbxMesh->max_face_triangles * Constant::k_triangleVertexCount;

	std::vector<std::uint32_t> l_triangleIndexList = {};

	l_triangleIndexList.resize(l_triangleIndexListSize);

	std::unordered_map < std::uint32_t, std::uint32_t> l_boneIndexPaletteIndexMap = {};

	l_boneIndexPaletteIndexMap.reserve(l_fbxSkinDeformer->clusters.count);

	for (auto l_faceIndex = 0ULL; l_faceIndex < l_fbxMesh->faces.count; ++l_faceIndex)
	{
		const auto& l_fbxFace = l_fbxMesh->faces.data[l_faceIndex];

		if (a_materialIndex != k_invalidMaterialIndex)
		{
			const auto& l_faceMaterialIndex = l_fbxMesh->face_material.data[l_faceIndex];

			FWK_ASSERT_RETURN_VALUE_IF(l_faceMaterialIndex >= l_fbxMesh->materials.count, "Faceが参照するMaterialIndexが範囲外です。", false);

			if (l_faceMaterialIndex != a_materialIndex) { continue; }
		}

		const auto l_triangleCount = ufbx_triangulate_face(l_triangleIndexList.data(),
														   l_triangleIndexList.size(),
														   l_fbxMesh,
														   l_fbxFace);

		for (auto l_triangleIndex = 0ULL; l_triangleIndex < l_triangleCount; ++l_triangleIndex)
		{
			for (auto l_vertexIndex = 0U; l_vertexIndex < Constant::k_triangleVertexCount; ++l_vertexIndex)
			{
				const auto& l_indexOffset    = (l_triangleIndex * Constant::k_triangleVertexCount) + l_vertexIndex;
				const auto  l_fbxVertexIndex = l_triangleIndexList[l_indexOffset];

				FWK_ASSERT_RETURN_VALUE_IF(l_fbxVertexIndex >= l_fbxMesh->vertex_indices.count, "三角形化後のVertexIndexが範囲外です。", false);

				SkeletalAnimationModelRecord::ModelVertex l_modelVertex = {};

				// 頂点はMeshローカル空間のまま保持する
				l_modelVertex.m_position = FetchLocalVertexPosition(l_fbxMesh, l_fbxVertexIndex);
				l_modelVertex.m_uv       = FetchVertexUV           (l_fbxMesh, l_fbxVertexIndex);
				l_modelVertex.m_normal   = FetchLocalVertexNormal  (l_fbxMesh, l_fbxVertexIndex);
				l_modelVertex.m_tangent  = FetchLocalVertexTangent (l_fbxMesh, l_fbxVertexIndex);

				FWK_ASSERT_RETURN_VALUE_IF(!ApplyModelVertexBoneInfluence(a_boneNodeIndexMap,
																		  l_fbxMesh,
																		  l_fbxSkinDeformer,
																		  l_fbxVertexIndex,
																		  l_boneIndexPaletteIndexMap,
																		  a_modelMesh.m_bonePaletteList,
																		  l_modelVertex),
																		  "ModelVertexへのBoneInfluence適用に失敗しました。",
																		  false);

				// 現在は重複頂点を削除せずに三角形頂点をそのまま追加する
				a_modelMesh.m_modelVertexList.emplace_back(l_modelVertex);
				a_modelMesh.m_indexList.emplace_back      (static_cast<std::uint32_t>(a_modelMesh.m_indexList.size()));
			}
		}
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::NormalizeModelVertexBoneWeight(SkeletalAnimationModelRecord::ModelVertex& a_modelVertex) const
{
	const auto l_boneWeightSum = a_modelVertex.m_boneWeight.x + a_modelVertex.m_boneWeight.y + a_modelVertex.m_boneWeight.z + a_modelVertex.m_boneWeight.w;

	FWK_ASSERT_RETURN_VALUE_IF(l_boneWeightSum <= k_emptyBoneWeight, "ModelVertexのBoneWeight合計が0以下です。", false);

	a_modelVertex.m_boneWeight.x /= l_boneWeightSum;
	a_modelVertex.m_boneWeight.y /= l_boneWeightSum;
	a_modelVertex.m_boneWeight.z /= l_boneWeightSum;
	a_modelVertex.m_boneWeight.w /= l_boneWeightSum;

	return true;
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexPosition(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、ローカル頂点座標の取得に失敗しました。", {});

	const auto& l_localPosition = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_position, a_vertexIndex);

	return Utility::ConvertUFBXVector3ToVector3(l_localPosition);
}
FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexNormal(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、ローカル頂点法線の取得に失敗しました。", {});

	if (!a_fbxMesh->vertex_normal.exists) { return {}; }

	const auto& l_localNormal      = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_normal, a_vertexIndex);
	const auto& l_normalizedNormal = ufbx_vec3_normalize (l_localNormal);

	return Utility::ConvertUFBXVector3ToVector3(l_normalizedNormal);
}
FWK::TypeAlias::Math::Vector4 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexTangent(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、ローカル頂点接線の取得に失敗しました。", {});

	ufbx_vec3 l_localTangent = { k_defaultTangentX, k_defaultTangentY, k_defaultTangentZ };

	if (a_fbxMesh->vertex_tangent.exists)
	{
		l_localTangent = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_tangent, a_vertexIndex);
		l_localTangent = ufbx_vec3_normalize (l_localTangent);
	}

	return TypeAlias::Math::Vector4
	{
		static_cast<float>(l_localTangent.x),
		static_cast<float>(l_localTangent.y),
		static_cast<float>(l_localTangent.z),
		k_defaultTangentW
	};
}