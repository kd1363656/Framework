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
	// SceneRoot直下のBoneはrootを持たない
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
FWK::Graphics::SkeletalAnimationModelRecord::ModelKeyFrame FWK::Graphics::SkeletalAnimationModelFBXLoader::CreateModelKeyFrame(const ufbx_baked_node* a_fbxBakedNode, const double& a_timeSecond) const
{
	SkeletalAnimationModelRecord::ModelKeyFrame l_modelKeyFrame = {};

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBakedNode, "ModelKeyFrameへ変換するufbx_baked_nodeがnullptrです。", l_modelKeyFrame);

	const auto& l_fbxScale       = ufbx_evaluate_baked_vec3(a_fbxBakedNode->scale_keys,       a_timeSecond);
	const auto& l_fbxRotation    = ufbx_evaluate_baked_quat(a_fbxBakedNode->rotation_keys,    a_timeSecond);
	const auto& l_fbxTranslation = ufbx_evaluate_baked_vec3(a_fbxBakedNode->translation_keys, a_timeSecond);

	l_modelKeyFrame.m_scale       = Utility::ConvertUFBXVector3ToVector3(l_fbxScale);
	l_modelKeyFrame.m_rotation    = ConvertUFBXQuaternionToQuaternion   (l_fbxRotation);
	l_modelKeyFrame.m_translation = Utility::ConvertUFBXVector3ToVector3(l_fbxTranslation);
	l_modelKeyFrame.m_timeSecond  = static_cast<float>                  (a_timeSecond);

	return l_modelKeyFrame;
}
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CreateModelBoneMotionTrack(const ufbx_baked_node*                                    a_fbxBakedNode, 
	                                                                             const double&                                             a_animationDurationSecond, 
	                                                                             const double&                                             a_animationFrameRate, 
	                                                                             const std::uint32_t                                       a_boneIndex, 
	                                                                                   SkeletalAnimationModelRecord::ModelBoneMotionTrack& a_modelBoneMotionTrack) const
{
	a_modelBoneMotionTrack = {};

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBakedNode,                                        "ModelBoneMotionTrackへ変換するufbx_baked_nodeがnullptrです。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animationDurationSecond < SkeletalAnimationModelRecord::k_initialAnimationDurationSecond, "Animation再生時間が0未満です。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animationFrameRate <= k_invalidAnimationFrameRate,     "AnimationFrameRateが0以下です。", false);

	FWK_ASSERT_RETURN_VALUE_IF(a_fbxBakedNode->scale_keys.count == k_emptyUFBXElementCount,       "Bake済みScaleKeyが存在しません。",       false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxBakedNode->rotation_keys.count == k_emptyUFBXElementCount,    "Bake済みRotationKeyが存在しません。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxBakedNode->translation_keys.count == k_emptyUFBXElementCount, "Bake済みTranslationKeyが存在しません。", false);

	const auto& l_animationFrameCount = static_cast<std::uint64_t>(std::ceil(a_animationDurationSecond * a_animationFrameRate));
	const auto& l_keyFrameCount       = l_animationFrameCount + k_animationTerminalKeyFrameCount;

	a_modelBoneMotionTrack.m_boneIndex = a_boneIndex;

	a_modelBoneMotionTrack.m_keyFrameList.clear  ();
	a_modelBoneMotionTrack.m_keyFrameList.reserve(l_keyFrameCount);

	for (auto l_keyFrameIndex = 0ULL; l_keyFrameIndex < l_keyFrameCount; ++l_keyFrameIndex)
	{
		double l_keyFrameTimeSecond = static_cast<double>(l_keyFrameIndex) / a_animationFrameRate;

		// 最後のSampleがAnimation終了時間を超える場合は終了時間へ固定する
		if (l_keyFrameTimeSecond > a_animationDurationSecond)
		{
			l_keyFrameTimeSecond = a_animationDurationSecond;
		}

		auto l_modelKeyFrame = CreateModelKeyFrame(a_fbxBakedNode, l_keyFrameTimeSecond);

		a_modelBoneMotionTrack.m_keyFrameList.emplace_back(l_modelKeyFrame);
	}

	return true;
}
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CreateModelMotionSequenceFromBakedAnimation(const BoneNodeIndexMap&                                  a_boneNodeIndexMap, 
	                                                                                             const ufbx_scene*                                        a_fbxScene, 
	                                                                                             const ufbx_baked_anim*                                   a_fbxBakedAnimation, 
	                                                                                                   SkeletalAnimationModelRecord::ModelMotionSequence& a_modelMotionSequence) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,          "Bake済みAnimationを変換するufbx_sceneがnullptrです。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBakedAnimation, "変換するufbx_baked_animがnullptrです。",                 false);
	FWK_ASSERT_RETURN_VALUE_IF(a_modelMotionSequence.m_frameRate <= k_invalidAnimationFrameRate, "MotionSequenceのFrameRateが0以下です。", false);

	const auto& l_animationDurationSecond = a_fbxBakedAnimation->playback_duration;
	const auto& l_animationFrameRate      = static_cast<double>(a_modelMotionSequence.m_frameRate);

	FWK_ASSERT_RETURN_VALUE_IF(l_animationDurationSecond < SkeletalAnimationModelRecord::k_initialAnimationDurationSecond, "Bake済みAnimationの再生時間が0未満です。", false);

	a_modelMotionSequence.m_durationSecond = static_cast<float>(l_animationDurationSecond);

	a_modelMotionSequence.m_boneMotionTrackList.clear  ();
	a_modelMotionSequence.m_boneMotionTrackList.reserve(a_fbxBakedAnimation->nodes.count);

	for (auto l_bakedNodeIndex = 0ULL; l_bakedNodeIndex < a_fbxBakedAnimation->nodes.count; ++l_bakedNodeIndex)
	{
		const auto& l_fbxBakedNode = a_fbxBakedAnimation->nodes.data[l_bakedNodeIndex];

		FWK_ASSERT_RETURN_VALUE_IF(l_fbxBakedNode.typed_id >= a_fbxScene->nodes.count, "Bake済みNodeのTypedIDがufbx_scene::nodesの範囲外です。", false);

		const auto* l_fbxBoneNode = a_fbxScene->nodes.data[l_fbxBakedNode.typed_id];

		FWK_ASSERT_RETURN_VALUE_IF(!l_fbxBoneNode, "Bake済みAnimationが参照するufbx_nodeがnullptrです。", false);

		const auto& l_boneNodeIndexITR = a_boneNodeIndexMap.find(l_fbxBoneNode);

		// Cameraや通常Objectなど、Skeletonに含まれないNodeのAnimationは保存しない
		if (l_boneNodeIndexITR == a_boneNodeIndexMap.end()) { continue; }

		SkeletalAnimationModelRecord::ModelBoneMotionTrack l_modelBoneMotionTrack = {};

		FWK_ASSERT_RETURN_VALUE_IF(!CreateModelBoneMotionTrack(&l_fbxBakedNode,
															   l_animationDurationSecond,
															   l_animationFrameRate,
															   l_boneNodeIndexITR->second,
															   l_modelBoneMotionTrack),
															   "Bake済みNodeからModelBoneMotionTrackの作成に失敗しました。",
															   false);

		a_modelMotionSequence.m_boneMotionTrackList.emplace_back(std::move(l_modelBoneMotionTrack));
	}

	return true;
}
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CreateModelMotionSequence(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_scene* a_fbxScene, const ufbx_anim_stack* a_fbxAnimationStack, SkeletalAnimationModelRecord::ModelMotionSequence& a_modelMotionSequence) const
{
	a_modelMotionSequence = {};

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,                "ModelMotionSequenceへ変換するufbx_sceneがnullptrです。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxAnimationStack,       "ModelMotionSequenceへ変換するufbx_anim_stackがnullptrです。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxAnimationStack->anim, "AnimationStackにufbx_animが存在しません。",                   false);

	double l_animationFrameRate = a_fbxScene->settings.frames_per_second;

	// FBXにFrameRateが設定されていない場合はFrameworkの基底値を使用する
	if (l_animationFrameRate <= k_invalidAnimationFrameRate) 
	{
		l_animationFrameRate = static_cast<double>(SkeletalAnimationModelRecord::k_defaultAnimationFrameRate);
	}

	ufbx_bake_opts l_bakeOptions = {};

	// RuntimeではAniamtionを0秒から再生する
	l_bakeOptions.trim_start_time = true;

	// FBXの非線形Animationを指定FrameRateeでBakeする
	l_bakeOptions.resample_rate = l_animationFrameRate;

	ufbx_error l_error = {};

	auto* l_fbxBakedAnimation = ufbx_bake_anim(a_fbxScene,
	                                           a_fbxAnimationStack->anim,
											   &l_bakeOptions,
											   &l_error );

	FWK_ASSERT_RETURN_VALUE_IF(!l_fbxBakedAnimation, "ufbx_bake_animによるAnimationのBakeに失敗しました。", false);

	a_modelMotionSequence.m_motionName = ConvertUFBXStringToWString(a_fbxAnimationStack->name);
	a_modelMotionSequence.m_frameRate  = static_cast<float>        (l_animationFrameRate);

	// Bake済みAnimationからMotionSequenceを作成する
	const auto l_isCreated = CreateModelMotionSequenceFromBakedAnimation(a_boneNodeIndexMap,
                                                                         a_fbxScene,
		                                                                 l_fbxBakedAnimation,
		                                                                 a_modelMotionSequence);
	
	// ufbx_bake_anim()で確保されたAnimationを明示的に解放する
	ufbx_free_baked_anim(l_fbxBakedAnimation);

	FWK_ASSERT_RETURN_VALUE_IF(!l_isCreated, "Bake済みAnimationからModelMotionSequenceの作成に失敗しました。", false);

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

	// 加算によるOverflowを避けるため、まず開始Indexだけを確認する
	FWK_ASSERT_RETURN_VALUE_IF(l_fbxSkinVertex.weight_begin > a_fbxSkinDeformer->weights.count, "SkinWeightの開始Indexが範囲外です。", false);

	const auto l_remainingSkinWeightCount = a_fbxSkinDeformer->weights.count - l_fbxSkinVertex.weight_begin;

	FWK_ASSERT_RETURN_VALUE_IF(l_fbxSkinVertex.num_weights > l_remainingSkinWeightCount, "SkinWeight配列の参照範囲が不正です。", false);
	
	a_modelVertex.m_bonePaletteIndex0 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndex1 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndex2 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_bonePaletteIndex3 = SkeletalAnimationModelRecord::k_invalidPaletteIndex;
	a_modelVertex.m_boneWeight        = {};

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
			a_modelVertex.m_boneWeight.z      = a_boneWeight;
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
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,                                        "ufbx_sceneが無効のため、ModelDataの抽出に失敗しました。",            false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxScene->nodes.count == k_emptyUFBXElementCount, "FBXシーン内にNodeが存在しないため、ModelDataの抽出に失敗しました。", false);

	a_modelData.m_modelMeshList.clear     ();
	a_modelData.m_boneList.clear          ();
	a_modelData.m_motionSequenceList.clear();

	BoneNodeIndexMap l_boneNodeIndexMap = {};

	// Meshより先にBoneIndexを確定する
	FWK_ASSERT_RETURN_VALUE_IF(!ExtractModelBoneList(a_fbxScene, l_boneNodeIndexMap, a_modelData.m_boneList), "FBXシーンからModelBoneListの抽出に失敗しました。", false);

	for (auto l_nodeIndex = 0ULL; l_nodeIndex < a_fbxScene->nodes.count; ++l_nodeIndex)
	{
		const auto* l_fbxNode = a_fbxScene->nodes.data[l_nodeIndex];

		if (!l_fbxNode) { continue; }

		const auto& l_fbxMesh = l_fbxNode->mesh;

		if (!l_fbxMesh) { continue; }

		// SkinDeformerを持たないMeshは対象外
		if (l_fbxMesh->skin_deformers.count == k_emptyUFBXElementCount) { continue; }

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
	FWK_ASSERT_RETURN_VALUE_IF(!ExtractModelMotionSequenceList(l_boneNodeIndexMap, a_fbxScene, a_modelData.m_motionSequenceList), "FBXシーンからMotionSequenceListの抽出に失敗しました。", false);

	return true;
}
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMeshList(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_node* a_fbxNode, std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const
{
	a_modelMeshList.clear();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode,       "ufbx_nodeがnullptrのため、ModelMeshListの抽出に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode->mesh, "ufbx_nodeにMeshが存在しないため、ModelMeshListの抽出に失敗しました。", false);

	const auto* l_fbxMesh = a_fbxNode->mesh;

	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->skin_deformers.count != k_supportedSkinDeformerCount, "一つのufbx_meshに設定されたSkinDeformer数が1個ではありません。", false);

	const auto* l_fbxSkinDeformer = l_fbxMesh->skin_deformers.data[k_initialSkinDeformerIndex];

	FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinDeformer, "ufbx_skin_deformerがnullptrです。", false);

	const bool l_isSupportedSkinningMethod = l_fbxSkinDeformer->skinning_method == UFBX_SKINNING_METHOD_LINEAR || 
											 l_fbxSkinDeformer->skinning_method == UFBX_SKINNING_METHOD_RIGID; 

	FWK_ASSERT_RETURN_VALUE_IF(!l_isSupportedSkinningMethod, "LinearまたはRigid以外のSkinningMethodには対応していません。", false);

	// Materialがない場合は全Faceを一つのModelMeshへ変換する
	if (l_fbxMesh->materials.count == k_emptyUFBXElementCount)
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
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelBoneList(const ufbx_scene* a_fbxScene, BoneNodeIndexMap& a_boneNodeIndexMap, std::vector<SkeletalAnimationModelRecord::ModelBone>& a_modelBoneList) const
{
	a_boneNodeIndexMap.clear();
	a_modelBoneList.clear   ();

	std::vector<const ufbx_node*> l_modelBoneNodeList = {};

	FWK_ASSERT_RETURN_VALUE_IF(!CollectModelBoneNodes(a_fbxScene, l_modelBoneNodeList),          "FBXシーンからBoneNodeの収集に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!CreateBoneNodeIndexMap(l_modelBoneNodeList, a_boneNodeIndexMap), "BoneNodeIndexMapの作成に失敗しました。",      false);

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
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMeshByMaterial(const std::size_t&                             a_materialIndex, 
																				const ufbx_node*                               a_fbxNode, 
																			    const BoneNodeIndexMap&                        a_boneNodeIndexMap,		
																					  SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const
{
	a_modelMesh.m_modelVertexList.clear();
	a_modelMesh.m_bonePaletteList.clear();
	a_modelMesh.m_indexList.clear      ();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode,       "ufbx_nodeがnullptrのため、Material別ModelMeshの抽出に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode->mesh, "ufbx_nodeにMeshが存在しないため、Material別ModelMeshの抽出に失敗しました。", false);

	const auto* l_fbxMesh = a_fbxNode->mesh;

	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->skin_deformers.count != k_supportedSkinDeformerCount,  "一つのufbx_meshに設定されたSkinDeformer数が1個ではありません。", false);
	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->faces.count == k_emptyUFBXElementCount,                "三角形化できるFaceが存在しません。",                             false);
	FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->max_face_triangles == Constant::k_emptyModelMeshCount, "三角形化できるFaceが存在しません。",                             false);

	FWK_ASSERT_RETURN_VALUE_IF(a_materialIndex != k_invalidMaterialIndex &&
							   l_fbxMesh->face_material.count != l_fbxMesh->faces.count,
							   "face_material数とFace数が一致しません。",
							   false);

	FWK_ASSERT_RETURN_VALUE_IF(a_materialIndex != k_invalidMaterialIndex &&
						       a_materialIndex >= l_fbxMesh->materials.count, 
						       "MateirlaIndexが範囲外です。",
						       false);

	const auto* l_fbxSkinDeformer = l_fbxMesh->skin_deformers.data[k_initialSkinDeformerIndex];

	FWK_ASSERT_RETURN_VALUE_IF(!l_fbxSkinDeformer, "ufbx_skin_deformerがnullptrです。", false);

	const auto& l_triangleIndexListSize = l_fbxMesh->max_face_triangles * Constant::k_triangleVertexCount;

	std::vector<std::uint32_t> l_triangleIndexList = {};

	l_triangleIndexList.resize(l_triangleIndexListSize);

	BoneIndexPaletteIndexMap l_boneIndexPaletteIndexMap = {};

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
bool FWK::Graphics::SkeletalAnimationModelFBXLoader::ExtractModelMotionSequenceList(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_scene* a_fbxScene, std::vector<SkeletalAnimationModelRecord::ModelMotionSequence>& a_modelMotionSequenceList) const
{
	a_modelMotionSequenceList.clear();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene, "MotionSequenceListを抽出するufbx_sceneがnullptrです。", false);

	// Animationを持たないSkeletalAnimationModelも正常なModelとして扱う
	if (a_fbxScene->anim_stacks.count == k_emptyUFBXElementCount) { return true; }

	a_modelMotionSequenceList.reserve(a_fbxScene->anim_stacks.count);

	for (auto l_animationStackIndex = 0ULL; l_animationStackIndex < a_fbxScene->anim_stacks.count; ++l_animationStackIndex)
	{
		const auto* l_fbxAnimationStack = a_fbxScene->anim_stacks.data[l_animationStackIndex];

		FWK_ASSERT_RETURN_VALUE_IF(!l_fbxAnimationStack, "ufbx_scene内のufbx_anim_stackがnullptrです。", false);

		SkeletalAnimationModelRecord::ModelMotionSequence l_modelMotionSequence = {};

		FWK_ASSERT_RETURN_VALUE_IF(!CreateModelMotionSequence(a_boneNodeIndexMap,
															  a_fbxScene,
															  l_fbxAnimationStack,
															  l_modelMotionSequence),
															  "ufbx_anim_stackからModelMotionSequenceの作成に失敗しました。",
															  false);

		// Skeleton内のBoneを動かさないAnimationStackは保存しない
		if (l_modelMotionSequence.m_boneMotionTrackList.empty()) { continue; }

		a_modelMotionSequenceList.emplace_back(std::move(l_modelMotionSequence));
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXLoader::CollectModelBoneNodes(const ufbx_scene* a_fbxScene, std::vector<const ufbx_node*>& a_modelBoneNodeList) const
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
		if (l_fbxMesh->skin_deformers.count == k_emptyUFBXElementCount) { continue; }

		FWK_ASSERT_RETURN_VALUE_IF(l_fbxMesh->skin_deformers.count != k_supportedSkinDeformerCount, "一つのufbx_meshに設定されたSkinDeformer数が1個ではありません。", false);

		const auto* l_fbxSkinDeformer = l_fbxMesh->skin_deformers.data[k_initialSkinDeformerIndex];

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

FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationModelFBXLoader::FetchLocalVertexPosition(const ufbx_mesh * a_fbxMesh, const std::uint32_t a_vertexIndex) const
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

FWK::TypeAlias::Math::Matrix FWK::Graphics::SkeletalAnimationModelFBXLoader::ConvertUFBXMatrixToMatrix(const ufbx_matrix& a_fbxMatrix) const
{
	// UFBXの列ベクトル行列をSimpleMathの行ベクトル行列へ転置する
	return TypeAlias::Math::Matrix
	{
		static_cast<float>(a_fbxMatrix.m00),
		static_cast<float>(a_fbxMatrix.m10),
		static_cast<float>(a_fbxMatrix.m20),
		k_affineMatrixAxisW,

		static_cast<float>(a_fbxMatrix.m01),
		static_cast<float>(a_fbxMatrix.m11),
		static_cast<float>(a_fbxMatrix.m21),
		k_affineMatrixAxisW,

		static_cast<float>(a_fbxMatrix.m02),
		static_cast<float>(a_fbxMatrix.m12),
		static_cast<float>(a_fbxMatrix.m22),
		k_affineMatrixAxisW,

		static_cast<float>(a_fbxMatrix.m03),
		static_cast<float>(a_fbxMatrix.m13),
		static_cast<float>(a_fbxMatrix.m23),
		k_affineMatrixTranslationW
	};
}
FWK::TypeAlias::Math::Quaternion FWK::Graphics::SkeletalAnimationModelFBXLoader::ConvertUFBXQuaternionToQuaternion(const ufbx_quat& a_fbxQuaternion) const
{
	const auto& l_normalizedQuaternion = ufbx_quat_normalize(a_fbxQuaternion);

	return TypeAlias::Math::Quaternion
	{
		static_cast<float>(l_normalizedQuaternion.x),
		static_cast<float>(l_normalizedQuaternion.y),
		static_cast<float>(l_normalizedQuaternion.z),
		static_cast<float>(l_normalizedQuaternion.w)
	};
}