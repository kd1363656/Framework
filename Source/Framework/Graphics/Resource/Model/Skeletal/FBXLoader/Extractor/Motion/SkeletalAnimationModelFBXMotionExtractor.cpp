#include "SkeletalAnimationModelFBXMotionExtractor.h"

bool FWK::Graphics::SkeletalAnimationModelFBXMotionExtractor::ExtractModelMotionSequenceList(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, const ufbx_scene* a_fbxScene, std::vector<SkeletalAnimationModelRecord::ModelMotionSequence>& a_modelMotionSequenceList) const
{
	a_modelMotionSequenceList.clear();

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene, "MotionSequenceListを抽出するufbx_sceneがnullptrです。", false);

	// Animationを持たないSkeletalAnimationModelも正常なModelとして扱う
	if (a_fbxScene->anim_stacks.count == Constant::k_emptyUFBXElementCount) { return true; }

	// アニメーション数分予約しておく(ベクター配列の再確保の発生が起きないため効率がいい)
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

FWK::Graphics::SkeletalAnimationModelRecord::ModelKeyFrame FWK::Graphics::SkeletalAnimationModelFBXMotionExtractor::CreateModelKeyFrame(const ufbx_baked_node* a_fbxBakedNode, const double& a_timeSecond) const
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

bool FWK::Graphics::SkeletalAnimationModelFBXMotionExtractor::CreateModelBoneMotionTrack(const ufbx_baked_node*                                    a_fbxBakedNode, 
	                                                                                     const double&                                             a_animationDurationSecond, 
	                                                                                     const double&                                             a_animationFrameRate, 
	                                                                                     const std::uint32_t                                       a_boneIndex, 
	                                                                                           SkeletalAnimationModelRecord::ModelBoneMotionTrack& a_modelBoneMotionTrack) const
{
	a_modelBoneMotionTrack = {};

	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBakedNode,                                        "ModelBoneMotionTrackへ変換するufbx_baked_nodeがnullptrです。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animationDurationSecond < SkeletalAnimationModelRecord::k_initialAnimationDurationSecond, "Animation再生時間が0未満です。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animationFrameRate <= k_invalidAnimationFrameRate,     "AnimationFrameRateが0以下です。", false);

	FWK_ASSERT_RETURN_VALUE_IF(a_fbxBakedNode->scale_keys.count == Constant::k_emptyUFBXElementCount,       "Bake済みScaleKeyが存在しません。",       false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxBakedNode->rotation_keys.count == Constant::k_emptyUFBXElementCount,    "Bake済みRotationKeyが存在しません。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(a_fbxBakedNode->translation_keys.count == Constant::k_emptyUFBXElementCount, "Bake済みTranslationKeyが存在しません。", false);

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

bool FWK::Graphics::SkeletalAnimationModelFBXMotionExtractor::CreateModelMotionSequenceFromBakedAnimation(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, 
	                                                                                                      const ufbx_scene*                                          a_fbxScene,
	                                                                                                      const ufbx_baked_anim*                                     a_fbxBakedAnimation, 
	                                                                                                            SkeletalAnimationModelRecord::ModelMotionSequence&   a_modelMotionSequence) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,          "Bake済みAnimationを変換するufbx_sceneがnullptrです。",                               false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBakedAnimation, "変換するufbx_baked_animがnullptrです。",                                             false);
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

bool FWK::Graphics::SkeletalAnimationModelFBXMotionExtractor::CreateModelMotionSequence(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap,
	                                                                                    const ufbx_scene*                                          a_fbxScene, 
	                                                                                    const ufbx_anim_stack*                                     a_fbxAnimationStack, 
	                                                                                          SkeletalAnimationModelRecord::ModelMotionSequence&   a_modelMotionSequence) const
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

	a_modelMotionSequence.m_motionName = Utility::ConvertUFBXStringToWString(a_fbxAnimationStack->name);
	a_modelMotionSequence.m_frameRate  = static_cast<float>                 (l_animationFrameRate);

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

FWK::TypeAlias::Math::Quaternion FWK::Graphics::SkeletalAnimationModelFBXMotionExtractor::ConvertUFBXQuaternionToQuaternion(const ufbx_quat& a_fbxQuaternion) const
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