#include "SkeletalAnimationPoseEvaluator.h"

bool FWK::Graphics::SkeletalAnimationPoseEvaluator::Create(SkeletalAnimationModelRecord::ModelData& a_modelData)
{
    const auto& l_modelBoneList = a_modelData.m_boneList;

    FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(), "ModelBoneListが空のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    std::vector<std::vector<std::uint32_t>> l_boneMotionTrackIndexList     = {};
    std::vector<LocalTransform>             l_bindPoseLocalTransformList   = {};
    
    // 各BoneのBindPoseをLocalTransformとして保存する。
    // また、MotionIndexとBoneIndexからBoneMotionTrackIndexを直接取得できる
    // Animation検索用テーブルを作成する。
    // この検索用テーブルを作成しておくことで、
    // Animation再生中にBoneMotionTrackList全体を毎フレーム検索する必要がなくなる。    
    FWK_ASSERT_RETURN_VALUE_IF(!CreateAnimationLookupData(a_modelData, l_bindPoseLocalTransformList, l_boneMotionTrackIndexList), "Animation検索用Dataの作成に失敗しました。",                         false);
    FWK_ASSERT_RETURN_VALUE_IF(l_bindPoseLocalTransformList.size() != l_modelBoneList.size(),                                     "BindPoseLocalTransformListの要素数がModelBoneListと一致しません。", false);

    // Animationが再生されていない状態でもBindPoseを使用できるように、
    // BindPoseのGlobalBoneMatrixを作成する。
    // ModelBoneListは親Boneから子Boneの順番で格納されているため、
    // 配列の先頭から計算することで親BoneのGlobalMatrixを利用できる。    
    std::vector<TypeAlias::Math::Matrix>    l_bindPoseGlobalBoneMatrixList = {};

    l_bindPoseGlobalBoneMatrixList.resize(l_modelBoneList.size(), TypeAlias::Math::Matrix::Identity);

    for (auto l_boneIndex = k_firstBoneIndex; l_boneIndex < l_modelBoneList.size(); ++l_boneIndex)
    {
        const auto& l_modelBone           = l_modelBoneList             [l_boneIndex];
        const auto& l_bindLocalTransform  = l_bindPoseLocalTransformList[l_boneIndex];
        const auto& l_bindPoseLocalMatrix = CreateLocalMatrix(l_bindLocalTransform);
        const auto  l_parentBoneIndex     = l_modelBone.m_parentBoneIndex;

        if (l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex) 
        {
            l_bindPoseGlobalBoneMatrixList[l_boneIndex] = l_bindPoseLocalMatrix;

            continue;
        }

        FWK_ASSERT_RETURN_VALUE_IF(static_cast<std::size_t>(l_parentBoneIndex) >= l_boneIndex, "親Boneが子Boneより後ろへ格納されています。", false);

        // SimapleMathの行ベクトル方式では、LocalMatrix * ParentGlobalMatrixの順番でGlobalMatrixを作成する
        l_bindPoseGlobalBoneMatrixList[l_boneIndex] = l_bindPoseLocalMatrix * l_bindPoseGlobalBoneMatrixList[l_parentBoneIndex];
    }

    m_boneMotionTrackIndexList     = std::move(l_boneMotionTrackIndexList);
    m_bindPoseGlobalBoneMatrixList = std::move(l_bindPoseGlobalBoneMatrixList);
    m_bindPoseLocalTransformList   = std::move(l_bindPoseLocalTransformList);

    return true;
}

bool FWK::Graphics::SkeletalAnimationPoseEvaluator::EvaluatePose(const SkeletalAnimationModelRecord::ModelData& a_modelData, 
                                                                 const float                                    a_animationTimeSecond,
                                                                 const float                                    a_blendTargetAnimationTimeSecond, 
                                                                 const float                                    a_blendWeight, 
                                                                 const std::uint32_t                            a_motionIndex, 
                                                                 const std::uint32_t                            a_blendTargetMotionIndex, 
                                                                 const bool                                     a_isBlending, 
                                                                       std::vector<TypeAlias::Math::Matrix>&    a_globalBoneMatrixList) const
{
    const auto& l_modelBoneList      = a_modelData.m_boneList;
    const auto& l_motionSequenceList = a_modelData.m_motionSequenceList;

    FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(), "ModelBoneListが空のため、現在Poseを計算できません。", false);

	// BindPoseLocalTransformはBoneごとに1つ必要
	// 要素数が異なる場合はBoneIndexからBindPoseを取得できない
	FWK_ASSERT_RETURN_VALUE_IF(m_bindPoseLocalTransformList.size() != l_modelBoneList.size(), "BindPoseLocalTransformListの要素数がModelBoneListと一致しません。", false);

	// 各Motionに対して1つのBoneMotionTrack検索用配列を作成しているため
	// MotionSequenceListと同じ要素数でなければならない
	FWK_ASSERT_RETURN_VALUE_IF(m_boneMotionTrackIndexList.size() != l_motionSequenceList.size(), "BoneMotionTrackIndexListの要素数がMotionSequenceListと一致しません。", false);

    // GlobalBoneMatrixはBoneごとに1つ必要。
    // Create()でModelBoneListと同じ要素数に初期化している。
    FWK_ASSERT_RETURN_VALUE_IF(a_globalBoneMatrixList.size() != l_modelBoneList.size(), "GlobalBoneMatrixListの要素数がModelBoneListと一致しません。", false);

    const bool l_hasCurrentMotion = a_motionIndex != k_invalidMotionIndex;

    FWK_ASSERT_RETURN_VALUE_IF(l_hasCurrentMotion && 
                               a_motionIndex >= l_motionSequenceList.size(), 
                               "現在AnimationのMotionIndexがMotionSequenceListの範囲外です。",
                               false);

    if (a_isBlending)
    {
    	// Blend元となる現在Motionがなければ、
		// 2つのAnimationを補間できない。
		FWK_ASSERT_RETURN_VALUE_IF(!l_hasCurrentMotion,                          "現在Motionが存在しないためAnimationをBlendできません。",          false);
		FWK_ASSERT_RETURN_VALUE_IF(a_blendTargetMotionIndex == k_invalidMotionIndex,        "Blend先AnimationのMotionIndexが無効です。",                       false);
		FWK_ASSERT_RETURN_VALUE_IF(a_blendTargetMotionIndex >= l_motionSequenceList.size(), "Blend先AnimationのMotionIndexがMotionSequenceListの範囲外です。", false);

        FWK_ASSERT_RETURN_VALUE_IF(a_blendWeight < k_minInterpolationWeight ||
		                           a_blendWeight > k_maxInterpolationWeight,
		                           "AnimationのBlendWeightが0から1の範囲外です。",
		                           false);
    }

    for (auto l_boneIndex = k_firstBoneIndex; l_boneIndex < l_modelBoneList.size(); ++l_boneIndex)
    {
        const auto& l_modelBone = l_modelBoneList[l_boneIndex];

        // Motionが存在しない場合や、
		// BoneMotionTrackが存在しない場合に使用する初期値。
        LocalTransform l_localTransform = m_bindPoseLocalTransformList[l_boneIndex];

        // Motionが存在しない場合や、
		// BoneMotionTrackが存在しない場合に使用する初期値。
        if (l_hasCurrentMotion)
        {
            const auto& l_currentMotionSequence = l_motionSequenceList[a_motionIndex];

            // 現在Motionの再生時刻から、
            // このBoneのLocalTransformを取得する。
            l_localTransform = SampleLocalTransform(l_currentMotionSequence, 
                                                    a_animationTimeSecond,
                                                    a_motionIndex,
                                                    static_cast<std::uint32_t>(l_boneIndex));
        }

        if (a_isBlending)
        {
            const auto& l_blendTargetMotionSequence = l_motionSequenceList[a_blendTargetMotionIndex];

            // Blend先Motionから同じBoneのLocalTransformを取得する。
            const auto& l_blendTargetLocalTransform = SampleLocalTransform(l_blendTargetMotionSequence,
                                                                          a_blendTargetAnimationTimeSecond,
                                                                          a_blendTargetMotionIndex,
                                                                          static_cast<std::uint32_t>(l_boneIndex));

            
			// Matrix同士を直接補間すると、
			// Scale・Rotation・Translationを正しく分離できず
			// Boneが歪む可能性がある。
			// そのためLocalTransformの状態でBlendする。
            l_localTransform = InterpolateLocalTransform(l_localTransform, l_blendTargetLocalTransform, a_blendWeight);
        }

        // 補間後のLocalTransformから、
		// 親Boneに対するLocalMatrixを作成する。
        const auto& l_localMatrix     = CreateLocalMatrix(l_localTransform);
        const auto  l_parentBoneIndex = l_modelBone.m_parentBoneIndex;

        if (l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex)
        {
            // 親を持たないRootBoneでは、
			// LocalMatrixがそのままGlobalMatrixになる
            a_globalBoneMatrixList[l_boneIndex] = l_localMatrix;

            continue;
        }

        // 親Boneは子Boneより先に計算されている必要がある。
		// この検証により、未計算のGlobalMatrix参照も防ぐ。
        FWK_ASSERT_RETURN_VALUE_IF(static_cast<std::size_t>(l_parentBoneIndex) >= l_boneIndex, "親Boneが子Boneより後ろへ格納されています。", false);

        // SimpleMathの行ベクトル方式では、
		// LocalMatrix × ParentGlobalMatrixの順番でGlobalMatrixを作る。
        a_globalBoneMatrixList[l_boneIndex] = l_localMatrix * a_globalBoneMatrixList[l_parentBoneIndex];
    }
 
    return true;
}

bool FWK::Graphics::SkeletalAnimationPoseEvaluator::CreateAnimationLookupData(SkeletalAnimationModelRecord::ModelData& a_modelData, std::vector<LocalTransform>& a_bindPoseLocalTransformList, std::vector<std::vector<std::uint32_t>>& a_boneMotionTrackIndexList) const
{
    // 出力先に以前のデータが残っている可能性があるため、
	// 新しいModelの検索用データを作成する前に初期化する。
    a_bindPoseLocalTransformList.clear();
    a_boneMotionTrackIndexList.clear  ();

          auto& l_modelBoneList      = a_modelData.m_boneList;
	const auto& l_motionSequenceList = a_modelData.m_motionSequenceList;
	
    FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(),                                                                           "ModelBoneListが空のため、Animation検索用Dataを作成できません。",       false);
	FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.size()      > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()), "ModelBoneListの要素数がuint32_tで表現できる範囲を超えています。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(l_motionSequenceList.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()), "MotionSequenceListの要素数がuint32_tで表現できる範囲を超えています。", false);

    // BindPoseLocalTransformはBoneごとに1個作成するため、
	// ModelBoneListと同じ数の容量を事前に予約する
    a_bindPoseLocalTransformList.reserve(l_modelBoneList.size());

    for (auto l_boneIndex = k_firstBoneIndex; l_boneIndex < l_modelBoneList.size(); ++l_boneIndex)
    {
        auto& l_modelBone = l_modelBoneList[l_boneIndex];
        
        if (const auto  l_parentBoneIndex = l_modelBone.m_parentBoneIndex;
            l_parentBoneIndex != SkeletalAnimationModelRecord::k_invalidBoneIndex)
        {
            // 存在しない親Boneを参照していないか検証する。
            FWK_ASSERT_RETURN_VALUE_IF(static_cast<std::size_t>(l_parentBoneIndex) >= l_modelBoneList.size(), "ParentBoneIndexがModelBoneListの範囲外です。", false);

            // GlobalBoneMatrixは親BoneのGlobalBoneMatrixを利用して計算する。
            // そのため、親Boneが子Boneより前へ格納されている必要がある。
			FWK_ASSERT_RETURN_VALUE_IF(static_cast<std::size_t>(l_parentBoneIndex) >= l_boneIndex, "親Boneが子Boneより後ろへ格納されています。", false);
        }

        LocalTransform l_bindPoseLocalTransform = {};

        // BindPoseLocalMatrixを、
		// Scale・Rotation・Translationへ分解して保存する。
		// AnimationTrackが存在しないBoneでは、
		// ここで作成したBindPoseLocalTransformを使用する。
        FWK_ASSERT_RETURN_VALUE_IF(!l_modelBone.m_bindPoseLocalMatrix.Decompose(l_bindPoseLocalTransform.m_scale,
			                                                                    l_bindPoseLocalTransform.m_rotation,
			                                                                    l_bindPoseLocalTransform.m_translation),
                                                                                "BindPoseLocalMatrixをScale、Rotation、Translationへ分解できません。",
		                                                                        false);

        l_bindPoseLocalTransform.m_rotation.Normalize();

        a_bindPoseLocalTransformList.emplace_back(l_bindPoseLocalTransform);
    }

    a_boneMotionTrackIndexList.reserve(l_motionSequenceList.size());

    for (auto l_motionIndex = k_firstMotionIndex; l_motionIndex < l_motionSequenceList.size(); ++l_motionIndex)
    {
        const auto& l_motionSequence      = l_motionSequenceList[l_motionIndex];
        const auto& l_boneMotionTrackList = l_motionSequence.m_boneMotionTrackList;

        FWK_ASSERT_RETURN_VALUE_IF(l_motionSequence.m_durationSecond < SkeletalAnimationModelRecord::k_initialAnimationDurationSecond, "Motionの再生時間が0未満です。",                                         false);
		FWK_ASSERT_RETURN_VALUE_IF(l_boneMotionTrackList.empty(),                                                                      "BoneMotionTrackListが空です。",                                         false);
		FWK_ASSERT_RETURN_VALUE_IF(l_boneMotionTrackList.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()), "BoneMotionTrackListの要素数がuint32_tで表現できる範囲を超えています。", false);

        // Motion内にTrackが存在しないBoneは、
		// k_invalidBoneMotionTrackIndexのまま残る。
		// その場合、Pose計算ではBindPoseを使用する。
        std::vector<std::uint32_t> l_motionBoneMotionTrackIndexList = {};

        l_motionBoneMotionTrackIndexList.resize(l_modelBoneList.size(), k_invalidBoneMotionTrackIndex);

        for (auto l_boneMotionTrackIndex = k_firstBoneMotionTrackIndex; l_boneMotionTrackIndex < l_boneMotionTrackList.size(); ++l_boneMotionTrackIndex)
        {
            const auto& l_boneMotionTrack = l_boneMotionTrackList[l_boneMotionTrackIndex];
			const auto& l_keyFrameList    = l_boneMotionTrack.m_keyFrameList;
			const auto  l_boneIndex       = l_boneMotionTrack.m_boneIndex;

            // BoneMotionTrackが存在しないBoneを参照していないか検証する。
            FWK_ASSERT_RETURN_VALUE_IF(l_boneIndex >= l_modelBoneList.size(), "BoneMotionTrackのBoneIndexがModelBoneListの範囲外です。", false);

            // 同じMotion内で1つのBoneへ複数のTrackが割り当てられると、
			// どちらを再生するか判断できないためエラーとする。
			FWK_ASSERT_RETURN_VALUE_IF(l_motionBoneMotionTrackIndexList[l_boneIndex] != k_invalidBoneMotionTrackIndex, "同じBoneを参照するBoneMotionTrackが重複しています。", false);

            // TrackにKeyFrameが存在しなければ補間処理を行えない。
			FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameList.empty(), "BoneMotionTrackのKeyFrameListが空です。", false);

            // 全KeyFrameの時刻がMotionの有効時間内に存在するか検証する
            for (auto l_keyFrameIndex = k_firstKeyFrameIndex; l_keyFrameIndex < l_keyFrameList.size(); ++l_keyFrameIndex)
            {
                const auto l_keyFrameTimeSecond = l_keyFrameList[l_keyFrameIndex].m_timeSecond;

                FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameTimeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond, "KeyFrameの時刻が0未満です。",                      false);
				FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameTimeSecond > l_motionSequence.m_durationSecond,                          "KeyFrameの時刻がMotionの再生時間を超えています。", false);
            }

            // 時刻から直接算出したKeyFrameIndexの前後を安全に補間できるように、
            // KeyFrameListが時刻の昇順で並んでいるか検証する。
            for (auto l_keyFrameIndex = k_firstKeyFrameIndex + k_nextKeyFrameOffset; l_keyFrameIndex < l_keyFrameList.size(); ++l_keyFrameIndex)
            {
                const auto l_previousKeyFrameIndex = l_keyFrameIndex - k_nextKeyFrameOffset;

                FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameList[l_keyFrameIndex].m_timeSecond < l_keyFrameList[l_previousKeyFrameIndex].m_timeSecond, "KeyFrameListが時刻順に並んでいません。", false);
            }

            // BoneIndexからBoneMotionTrackIndexを直接取得できるように登録する。
            l_motionBoneMotionTrackIndexList[l_boneIndex] = static_cast<std::uint32_t>(l_boneMotionTrackIndex);
        }

        // 完成した1Motion分の検索用配列を保存する。
        a_boneMotionTrackIndexList.emplace_back(std::move(l_motionBoneMotionTrackIndexList));
    }

    return true;
}

FWK::TypeAlias::Math::Matrix FWK::Graphics::SkeletalAnimationPoseEvaluator::CreateLocalMatrix(const LocalTransform& a_localTransform) const
{
    // SimpleMathは行ベクトルとして使用するため
    // Scale -> Rotation -> Translationの順番で合成する
    return TypeAlias::Math::Matrix::CreateScale(a_localTransform.m_scale) * TypeAlias::Math::Matrix::CreateFromQuaternion(a_localTransform.m_rotation) * TypeAlias::Math::Matrix::CreateTranslation(a_localTransform.m_translation);
}

FWK::Graphics::SkeletalAnimationPoseEvaluator::LocalTransform FWK::Graphics::SkeletalAnimationPoseEvaluator::SampleLocalTransform(const SkeletalAnimationModelRecord::ModelMotionSequence& a_motionSequence, 
                                                                                                                                  const float         a_timeSecond, 
                                                                                                                                  const std::uint32_t a_motionIndex, 
                                                                                                                                  const std::uint32_t a_boneIndex) const
{
// BoneIndexが範囲外の場合は、
	// BindPoseにもアクセスできないため初期値を返す。
    FWK_ASSERT_RETURN_VALUE_IF(a_boneIndex >= m_bindPoseLocalTransformList.size(), "BoneIndexがBindPoseLocalTransformListの範囲外です。", {});

    const auto& l_bindPoseLocalTransform = m_bindPoseLocalTransformList[a_boneIndex];

    FWK_ASSERT_RETURN_VALUE_IF(a_timeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond, "AnimationTimeSecondが0未満です。", l_bindPoseLocalTransform);

    // Loopや再生時間内へのClampはAdvanceTime()で行う。
    FWK_ASSERT_RETURN_VALUE_IF(a_timeSecond                 > a_motionSequence.m_durationSecond,  "AnimationTimeSecondがMotionの再生時間を超えています。", l_bindPoseLocalTransform);
    FWK_ASSERT_RETURN_VALUE_IF(a_motionSequence.m_frameRate <= std::numeric_limits<float>::min(), "AnimationFrameRateが0以下です。",                       l_bindPoseLocalTransform);
	FWK_ASSERT_RETURN_VALUE_IF(a_motionIndex                == k_invalidMotionIndex,              "MotionIndexが無効です。",                               l_bindPoseLocalTransform);
	FWK_ASSERT_RETURN_VALUE_IF(a_motionIndex                >= m_boneMotionTrackIndexList.size(), "MotionIndexがBoneMotionTrackIndexListの範囲外です。",   l_bindPoseLocalTransform);

    // CreateAnimationLookupData()で作成した検索表から、
	// BoneMotionTrackIndexを直接取得する。
    // 配列の関係：
	// m_boneMotionTrackIndexList[MotionIndex][BoneIndex]
    const auto& l_motionBoneMotionTrackIndexList = m_boneMotionTrackIndexList[a_motionIndex];

    FWK_ASSERT_RETURN_VALUE_IF(a_boneIndex >= l_motionBoneMotionTrackIndexList.size(), "BoneIndexがMotionのBoneMotionTrackIndexListの範囲外です。", l_bindPoseLocalTransform);

    const auto l_boneMotionTrackIndex = l_motionBoneMotionTrackIndexList[a_boneIndex];

    // Motion内にこのBoneを動かすTrackが存在しない場合は、
    // BindPoseのLocalTransformを使用する。
    if (l_boneMotionTrackIndex == k_invalidBoneMotionTrackIndex) { return l_bindPoseLocalTransform; }

    const auto& l_boneMotionTrackList = a_motionSequence.m_boneMotionTrackList;

    FWK_ASSERT_RETURN_VALUE_IF(l_boneMotionTrackIndex >= l_boneMotionTrackList.size(), "BoneMotionTrackIndexがBoneMotionTrackListの範囲外です。", l_bindPoseLocalTransform);

    const auto& l_keyFrameList = l_boneMotionTrackList[l_boneMotionTrackIndex].m_keyFrameList;

    FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameList.empty(), "KeyFrameListが空のためLocalTransformを取得できません。", l_bindPoseLocalTransform);

    // KeyFrameが1つだけの場合は、
	// 再生時刻に関係なく同じPoseを使用する。
    if (l_keyFrameList.size() == k_singleKeyFrameCount)
    {
        const auto& l_keyFrame = l_keyFrameList[k_firstKeyFrameIndex];

        LocalTransform l_localTransform = {};

        l_localTransform.m_scale       = l_keyFrame.m_scale;
        l_localTransform.m_rotation    = l_keyFrame.m_rotation;
        l_localTransform.m_translation = l_keyFrame.m_translation;

        l_localTransform.m_rotation.Normalize();

        return l_localTransform;
    }

    const auto& l_lastKeyFrameIndex = l_keyFrameList.size() - k_nextKeyFrameOffset;

    // Animationは一定FrameRateでBakeされているため、
	// Animation時刻にFrameRateを掛ければ、
	// 現在時刻が存在するKeyFrame区間を直接求められる
	// 例：
	// AnimationTime = 0.25秒
	// FrameRate     = 30FPS
	// FramePosition = 7.5
	// この場合、KeyFrame[7]とKeyFrame[8]の間を補間する
    const auto& l_sampleFramePosition = a_timeSecond * a_motionSequence.m_frameRate;

    auto l_startKeyFrameIndex = k_firstKeyFrameIndex;

    // 異常なIndexへ変換しないように、
	// size_tへ変換する前に最後のKeyFrameIndexと比較する
    if (l_sampleFramePosition >= static_cast<float>(l_lastKeyFrameIndex))
    {
        l_startKeyFrameIndex = l_lastKeyFrameIndex;
    }
    else
    {
        // Animation時刻とFrameRateは0以上であることを検証済みなので、
		// size_tへの変換によって小数部分を切り捨てる
        l_startKeyFrameIndex = static_cast<std::size_t>(l_sampleFramePosition);
    }

    auto l_endKeyFrameIndex = l_startKeyFrameIndex + k_nextKeyFrameOffset;

    // 最後のKeyFrameを超えないようにClampする
    if (l_endKeyFrameIndex > l_lastKeyFrameIndex) { l_endKeyFrameIndex = l_lastKeyFrameIndex; }

    const auto& l_startKeyFrame = l_keyFrameList[l_startKeyFrameIndex];

    LocalTransform l_startLocalTransform = {};

    l_startLocalTransform.m_scale       = l_startKeyFrame.m_scale;
    l_startLocalTransform.m_rotation    = l_startKeyFrame.m_rotation;
    l_startLocalTransform.m_translation = l_startKeyFrame.m_translation;

    l_startLocalTransform.m_rotation.Normalize();

    // 開始と終了が同じKeyFrameなら補間は不要
    if (l_startKeyFrameIndex == l_endKeyFrameIndex) { return l_startLocalTransform; }

    const auto& l_endKeyFrame = l_keyFrameList[l_endKeyFrameIndex];

    LocalTransform l_endLocalTransform = {};

    l_endLocalTransform.m_scale       = l_endKeyFrame.m_scale;
    l_endLocalTransform.m_rotation    = l_endKeyFrame.m_rotation;
    l_endLocalTransform.m_translation = l_endKeyFrame.m_translation;

    l_endLocalTransform.m_rotation.Normalize();

    const auto l_keyFrameTimeRange = l_endKeyFrame.m_timeSecond - l_startKeyFrame.m_timeSecond;

    // 最後のKeyFrameはMotion終了時刻へClampされる場合がある。
	// そのため補間率にはFramePositionの小数部分を直接使わず、
	// 実際に保存されているKeyFrame時刻から計算する
    if (l_keyFrameTimeRange <= k_minKeyFrameTimeRange) { return l_startLocalTransform; }

    const auto l_interpolationWeight = (a_timeSecond - l_startKeyFrame.m_timeSecond) / l_keyFrameTimeRange;

    return InterpolateLocalTransform(l_startLocalTransform, l_endLocalTransform, l_interpolationWeight);
}

FWK::Graphics::SkeletalAnimationPoseEvaluator::LocalTransform FWK::Graphics::SkeletalAnimationPoseEvaluator::InterpolateLocalTransform(const LocalTransform& a_startLocalTransform, const LocalTransform& a_endLocalTransform, const float a_interpolationWeight) const
{
// 補間率が0以下の場合は開始Transformをそのまま返す。
	// 範囲外の値を許可しないことで、意図しない外挿を防ぐ。
    if (a_interpolationWeight <= k_minInterpolationWeight) { return a_startLocalTransform; }

    // 補完率が1以上の場合は終了Transformをそのまま返す
    if (a_interpolationWeight >= k_maxInterpolationWeight) { return a_endLocalTransform; }

    LocalTransform l_interpolatedLocalTransform = {};

    // Scaleは各成分を線形補完する
    l_interpolatedLocalTransform.m_scale = TypeAlias::Math::Vector3::Lerp(a_startLocalTransform.m_scale, a_endLocalTransform.m_scale, a_interpolationWeight);

    // Quaternionの球面線形補間は、
	// 長さが1のQuaternionを前提としているため両方を正規化する
    auto l_startRotation = a_startLocalTransform.m_rotation;
    auto l_endRotation   = a_endLocalTransform.m_rotation;

    l_startRotation.Normalize();
    l_endRotation.Normalize  ();
    
    // RotationはQuaternionを単純に線形補間すると、
	// 回転速度が不均一になったりQuaternionの長さが崩れたりする。
	// そのため、回転補間にはSlerpを使用する
    l_interpolatedLocalTransform.m_rotation = TypeAlias::Math::Quaternion::Slerp(l_startRotation, l_endRotation, a_interpolationWeight);

    // Slerp後に正規化し、
    // 小さな浮動小数点誤差が行列へ蓄積されることを防ぐ
    l_interpolatedLocalTransform.m_rotation.Normalize();

    // Translationは各成分を線形補間する。
    l_interpolatedLocalTransform.m_translation = TypeAlias::Math::Vector3::Lerp(a_startLocalTransform.m_translation, a_endLocalTransform.m_translation, a_interpolationWeight);

    return l_interpolatedLocalTransform;
}