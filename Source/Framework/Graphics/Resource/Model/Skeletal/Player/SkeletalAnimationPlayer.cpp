#include "SkeletalAnimationPlayer.h"

bool FWK::Graphics::SkeletalAnimationPlayer::Create(const SkeletalAnimationModel& a_skeletalAnimationModel)
{
    FWK_ASSERT_RETURN_VALUE_IF(!a_skeletalAnimationModel.IsValid(), "SkeletalAnimationModelが無効のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    const auto& l_skeletalAnimationModelRecord = a_skeletalAnimationModel.GetREFSkeletalAnimationModelRecord().lock();
    
    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

          auto& l_modelData     = l_skeletalAnimationModelRecord->GetMutableREFModelData();
    const auto& l_modelBoneList = l_modelData.m_boneList;
    const auto& l_modelMeshList = l_modelData.m_modelMeshList;

    FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(), "ModelBoneListが空のため、SkeletalAnimationPlayerの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    // 各BoneのBindPoseをLocalTransformとして保存する。
    // また、MotionIndexとBoneIndexからBoneMotionTrackIndexを直接取得できる
    // Animation検索用テーブルを作成する。
    // この検索用テーブルを作成しておくことで、
    // Animation再生中にBoneMotionTrackList全体を毎フレーム検索する必要がなくなる。
    std::vector<LocalTransform>             l_bindPoseLocalTransformList = {};
    std::vector<std::vector<std::uint32_t>> l_boneMotionTrackIndexList   = {};

    FWK_ASSERT_RETURN_VALUE_IF(!CreateAnimationLookupData(l_modelData, l_bindPoseLocalTransformList, l_boneMotionTrackIndexList), "Animation検索用Dataの作成に失敗しました。", false);

    // Animationが再生されていない状態でもBindPoseを使用できるように、
    // BindPoseのGlobalBoneMatrixを作成する。
    // ModelBoneListは親Boneから子Boneの順番で格納されているため、
    // 配列の先頭から計算することで親BoneのGlobalMatrixを利用できる。
    std::vector<TypeAlias::Math::Matrix> l_bindPoseGlobalBoneMatrixList = {};

    l_bindPoseGlobalBoneMatrixList.resize(l_modelBoneList.size(), TypeAlias::Math::Matrix::Identity);

    for (auto l_boneIndex = k_firstBoneIndex; l_boneIndex < l_modelBoneList.size(); ++l_boneIndex)
    {
        const auto& l_modelBone          = l_modelBoneList             [l_boneIndex];
        const auto& l_bindLocalTransform = l_bindPoseLocalTransformList[l_boneIndex];

        const auto l_bindPoseLocalMatrix = CreateLocalMatrix(l_bindLocalTransform);
        const auto l_parentBoneIndex     = l_modelBone.m_parentBoneIndex;

        if (l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex) 
        {
            l_bindPoseGlobalBoneMatrixList[l_boneIndex] = l_bindPoseLocalMatrix;

            continue;
        }

        l_bindPoseGlobalBoneMatrixList[l_boneIndex] = l_bindPoseLocalMatrix * l_bindPoseGlobalBoneMatrixList[l_parentBoneIndex];
    }

    auto& l_graphicsManager = GraphicsManager::GetInstance();

    const auto& l_device   = l_graphicsManager.GetREFDevice  ();
    const auto& l_renderer = l_graphicsManager.GetREFRenderer();

    auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();

    const auto& l_gpuMemoryAllocator      = l_resourceContext.GetREFGPUMemoryAllocator            ();
          auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_frameResourceList = l_renderer.GetREFFrameResourceList();

    FWK_ASSERT_RETURN_VALUE_IF(l_frameResourceList.empty(), "FrameResourceListが空のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    // Playerの再作成途中で失敗しても、
    // 現在Playerが保持している正常なFrameDataを壊さないように、
    // 新しいFrameDataは一度ローカル変数へ作成する
    std::vector<FrameData> l_frameDataList = {};

    // フレームリソースの数だけ容量を予約
    l_frameDataList.reserve(l_frameResourceList.size());

    // 各FrameResourceで使用するBoneMatrixBufferと
    // MeshごとのSkinnedVertexBufferを作成する
    for (const auto& l_frameResource : l_frameResourceList)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameResource, "FrameResourceが無効のため、SkeletalAnimationPlayerのFrameData作成に失敗しました。", false);

        FrameData l_frameData = {};
        
		// CPUで計算したGlobalBoneMatrixをGPUへ渡すためのBufferを作成する。
		// BoneMatrixBufferはModel全体で1つ持ち、
		// Modelに含まれるBone数と同じ数のMatrixを格納する。
		// GPUのスキニングComputeShaderはこのBufferを読み込み、
		// 各頂点へLBSを適用する。
        if (!l_frameData.m_boneMatrixBuffer.Create<TypeAlias::Math::Matrix>(l_device, 
                                                                            l_gpuMemoryAllocator,
                                                                            l_modelBoneList.size(),
                                                                            l_cbvSRVUAVDescriptorPool))
        {
            // 作成済みBufferはローカルvectorが破棄される際に解放される。
            // Playerの既存メンバにはまだ反映していないため、
            // Playerの再作成途中で失敗しても以前の正常な状態は維持される。   
            FWK_ASSERT_RETURN_VALUE("BoneMatrix用DynamicRWStructuredBufferの作成に失敗しました。", false);
        }

        l_frameData.m_globalBoneMatrixList = l_bindPoseGlobalBoneMatrixList;

        // SkinnedVertexBufferはMeshごとに頂点数が異なるため、
	    // ModelMeshListと同じ数だけ作成する
        l_frameData.m_skinnedVertexBufferList.reserve(l_modelMeshList.size());

        for (const auto& l_modelMesh : l_modelMeshList)
        {
            FWK_ASSERT_RETURN_VALUE_IF(l_modelMesh.m_modelVertexList.empty(), "ModelVertexListが空のため、SkinnedVertexBufferの作成に失敗しました。", false);

            DynamicRWStructuredBuffer l_skinnedVertexBuffer = {};

            if (!l_skinnedVertexBuffer.Create<SkinnedVertexBufferElement>(l_device,
                                                                          l_gpuMemoryAllocator,
                                                                          l_modelMesh.m_modelVertexList.size(),
                                                                          l_cbvSRVUAVDescriptorPool))
            {
                FWK_ASSERT_RETURN_VALUE("SkinnedVertex用DynamicRWStructuredBufferの作成に失敗しました。", false);
            }

            l_frameData.m_skinnedVertexBufferList.emplace_back(std::move(l_skinnedVertexBuffer));
        }

        l_frameDataList.emplace_back(std::move(l_frameData));
    }

    // 全ての作成処理が成功してからメンバ変数へ反映する
    m_skeletalAnimationModelRecord = a_skeletalAnimationModel.GetREFSkeletalAnimationModelRecord();
    m_boneMotionTrackIndexList     = std::move                                                  (l_boneMotionTrackIndexList);
    m_frameDataList                = std::move                                                  (l_frameDataList);
    m_bindPoseLocalTransformList   = std::move                                                  (l_bindPoseLocalTransformList);
    
    // 新しいModelへ切り替えたため、
    // 以前のMotion再生状態を残さないように初期化する
    ResetPlaybackState();

    return true;
}

bool FWK::Graphics::SkeletalAnimationPlayer::PlayMotion(const std::uint32_t a_motionIndex, const bool a_isLoop, const float a_playbackSpeed)
{
    Animation l_animation = {};

    l_animation.m_motionIndex   = a_motionIndex;
    l_animation.m_playbackSpeed = a_playbackSpeed;
    l_animation.m_isLoop        = a_isLoop;

    // 負の再生速度が指定された場合は、
    // Motion先頭ではなくMotion終端から逆再生を開始する
    if (a_playbackSpeed < k_stoppedPlaybackSpeed) 
    {
        l_animation.m_startTimeSecond = FetchMotionDurationSecond(l_animation);
    }

    // PlayMotion()ではBlendを行わず
    // 指定Motionへ即時に切り替える
    l_animation.m_blendDurationSecond = Animation::k_initialBlendDurationSecond;

    return ApplyAnimation(l_animation);
}

void FWK::Graphics::SkeletalAnimationPlayer::AdvanceTime(const float a_deltaTime)
{
    FWK_ASSERT_RETURN_IF(a_deltaTime < FPSController::k_minDeltaTime, "DeltaTimeが0未満のため、Animationの再生時刻を更新できません。");

    // Motionが設定されていない場合はBindPose状態なので、
	// Animationの再生時刻を更新しない
	if (m_animation.m_motionIndex == Animation::k_invalidMotionIndex) { return; }

    // 現在Animationの再生時刻を更新する
    m_animationTimeSecond = CalculateAdvancedTimeSecond(m_animation, m_animationTimeSecond, a_deltaTime);

    if (!m_isBlending) { return; }

    // Blend先Animationは現在Animationとは異なる再生速度や
	// Loop設定を持てるため、別の再生時刻として更新する
	m_blendTargetAnimationTimeSecond = CalculateAdvancedTimeSecond(m_blendTargetAnimation, m_blendTargetAnimationTimeSecond, a_deltaTime);

    // Blend時間はMotionの再生速度とは関係なく、
	// 実際に経過したDeltaTimeによって進行させる
    m_blendElapsedSecond += a_deltaTime;

    if (m_blendElapsedSecond >= m_blendTargetAnimation.m_blendDurationSecond) 
    {
        CompleteAnimationBlend();
    }
}

bool FWK::Graphics::SkeletalAnimationPlayer::IsAnimationEnd() const
{
    // Motionが設定されいない場合は
    // 再生対象が存在しないため終了状態として扱う
    if (m_animation.m_motionIndex == Animation::k_invalidMotionIndex) { return true; }

    // Blend中はBlend先Animationへ移行している途中なので、
	// 現在Animationが終端へ到達していても終了扱いにしない
    if (m_isBlending) { return false; }

    // Loop Animationは終端または先頭へ到達しても、
	// 再生を継続するため終了状態にならない
    if (m_animation.m_isLoop) { return false; }

    const auto l_motionDurationSecond = FetchMotionDurationSecond(m_animation);

    // 負の再生速度ではMotion先頭へ到達した時点で終了する
    if (m_animation.m_playbackSpeed < k_stoppedPlaybackSpeed) { return m_animationTimeSecond <= SkeletalAnimationModelRecord::k_initialAnimationTimeSecond; }

    // 通常再生または停止状態では、
    // Motion終端へ到達した時点で終了する
    return m_animationTimeSecond >= l_motionDurationSecond;
}

void FWK::Graphics::SkeletalAnimationPlayer::Stop()
{
    // ModelRecordとBoneMatrixBufferは次のMotionでも使用するため保持し、
	// Animationの再生状態だけを初期化する
    ResetPlaybackState();
}

bool FWK::Graphics::SkeletalAnimationPlayer::ApplyAnimation(const Animation& a_animation)
{
    const auto& l_skeletalAnimationModelRecord = m_skeletalAnimationModelRecord.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、Animationを適用できません。", false);

    const auto& l_motionSequenceList = l_skeletalAnimationModelRecord->GetREFModelData().m_motionSequenceList;

    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex == Animation::k_invalidMotionIndex, "MotionIndexが無効なため、Animationを適用できません。",   false);
    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex >= l_motionSequenceList.size(),     "MotionIndexが範囲外のため、Animationを適用できません。", false);

    const auto l_motionDurationSecond = l_motionSequenceList[a_animation.m_motionIndex].m_durationSecond;

    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_startTimeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond,     "AnimationのStartTimeSecondが0未満のため、Animationを適用できません。",                      false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_startTimeSecond > l_motionDurationSecond,                                         "AnimationのStartTimeSecondがMotionの再生時間を超えているため、Animationを適用できません。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_blendDurationSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond, "AnimationのBlendDurationSecondが0未満のため、Animationを適用できません。",                  false);

    if (m_animation.m_motionIndex == Animation::k_invalidMotionIndex ||
        m_animation.m_motionIndex >= l_motionSequenceList.size()     ||
        a_animation.m_blendDurationSecond == Animation::k_initialBlendDurationSecond)
    {
        m_animation = a_animation;

        // 現在Animationへ移した後は、
        // このAnimation自身のBlend時間は使用しない。
        m_animation.m_blendDurationSecond = Animation::k_initialBlendDurationSecond;

        m_animationTimeSecond = a_animation.m_startTimeSecond;

        m_blendTargetAnimation           = {};
        m_blendTargetAnimationTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
        m_blendElapsedSecond             = k_initialBlendElapsedSecond;

        m_isBlending = false;

        return true;
    }

    // 現在のBlend結果を次のBlend元として保存するBufferはまだ存在しないため、
    // Blend中に別のBlendeを重ねることは禁止する
    // Blend時間が0秒の即時切り替えは、上の分岐で実行できる
    FWK_ASSERT_RETURN_VALUE_IF(m_isBlending, "AnimationのBlend中であるため、新しいAnimationのBlendを開始できません。", false);

    // 現在AnimationはBlend元として維持し、
	// 指定されたAnimationをBlend先として設定する
    m_blendTargetAnimation           = a_animation;
    m_blendTargetAnimationTimeSecond = a_animation.m_startTimeSecond;
    m_blendElapsedSecond             = k_initialBlendElapsedSecond;

    m_isBlending = true;

    return true;
}

const FWK::Graphics::SkeletalAnimationPlayer::FrameData* FWK::Graphics::SkeletalAnimationPlayer::FetchPTRCurrentFrameData() const
{
    const auto& l_graphicsManager           = GraphicsManager::GetInstance              ();
    const auto& l_renderer                  = l_graphicsManager.GetREFRenderer          ();
    const auto& l_currentFrameResourceIndex = l_renderer.GetREFCurrentFrameResourceIndex();

    FWK_ASSERT_RETURN_VALUE_IF(l_currentFrameResourceIndex >= m_frameDataList.size(), "CurrentFrameResourceIndexが範囲外のため、SkeletalAnimationPlayerのFrameDataを取得できません。", nullptr);

    return &m_frameDataList[l_currentFrameResourceIndex];
}

FWK::Graphics::SkeletalAnimationPlayer::FrameData* FWK::Graphics::SkeletalAnimationPlayer::FetchMutablePTRCurrentFrameData()
{
    const auto& l_graphicsManager           = GraphicsManager::GetInstance              ();
    const auto& l_renderer                  = l_graphicsManager.GetREFRenderer          ();
    const auto& l_currentFrameResourceIndex = l_renderer.GetREFCurrentFrameResourceIndex();

    FWK_ASSERT_RETURN_VALUE_IF(l_currentFrameResourceIndex >= m_frameDataList.size(), "CurrentFrameResourceIndexが範囲外のため、SkeletalAnimationPlayerのFrameDataを取得できません。", nullptr);

    return &m_frameDataList[l_currentFrameResourceIndex];
}

float FWK::Graphics::SkeletalAnimationPlayer::FetchVALBlendWeight() const
{
    if (!m_isBlending) { return k_initialBlendWeight; }

    const auto l_blendDurationSecond = m_blendTargetAnimation.m_blendDurationSecond;

    // ApplyAnimation()では0秒Blendを即時切り替えとして処理しているが、
	// 0除算を防ぐため、取得時にもBlend時間を確認する
    if (l_blendDurationSecond <= Animation::k_initialBlendDurationSecond) { return k_completeBlendWeight; }

    const auto l_blendWeight = m_blendElapsedSecond / l_blendDurationSecond;

    if (l_blendWeight <= k_initialBlendWeight)  { return k_initialBlendWeight; }
    if (l_blendWeight >= k_completeBlendWeight) { return k_completeBlendWeight; }

    return l_blendWeight;
}

bool FWK::Graphics::SkeletalAnimationPlayer::CreateAnimationLookupData(SkeletalAnimationModelRecord::ModelData& a_modelData, std::vector<LocalTransform>& a_bindPoseLocalTransformList, std::vector<std::vector<std::uint32_t>>& a_boneMotionTrackIndexList) const
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

    for (auto l_motionIndex = k_firstBoneIndex; l_motionIndex , l_motionSequenceList.size(); ++l_motionIndex)
    {
        const auto& l_motionSequence      = l_motionSequenceList[l_motionIndex];
        const auto& l_boneMotionTrackList = l_motionSequence.m_boneMotionTrackList;

        FWK_ASSERT_RETURN_VALUE_IF(!std::isfinite(l_motionSequence.m_durationSecond),                                                  "Motionの再生時間が有限値ではありません。",                              false);
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

                FWK_ASSERT_RETURN_VALUE_IF(!std::isfinite(l_keyFrameTimeSecond),                                              "KeyFrameの時刻が有限値ではありません。",           false);
				FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameTimeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond, "KeyFrameの時刻が0未満です。",                      false);
				FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameTimeSecond > l_motionSequence.m_durationSecond,                          "KeyFrameの時刻がMotionの再生時間を超えています。", false);
            }

            // 二分探索を正しく行えるように、
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
FWK::TypeAlias::Math::Matrix FWK::Graphics::SkeletalAnimationPlayer::CreateLocalMatrix(const LocalTransform& a_localTransform) const
{
    auto l_normalizedRotatin = a_localTransform.m_rotation;

    l_normalizedRotatin.Normalize();

    // SimpleMathは行ベクトルとして使用するため
    // Scale -> Rotation -> Translationの順番で合成する
    return TypeAlias::Math::Matrix::CreateScale(a_localTransform.m_scale) * TypeAlias::Math::Matrix::CreateFromQuaternion(a_localTransform.m_rotation) * TypeAlias::Math::Matrix::CreateTranslation(a_localTransform.m_translation);
}


bool FWK::Graphics::SkeletalAnimationPlayer::EvaluateCurrentPose()
{
    return false;
}

FWK::Graphics::SkeletalAnimationPlayer::LocalTransform FWK::Graphics::SkeletalAnimationPlayer::SampleLocalTransform(const SkeletalAnimationModelRecord::ModelMotionSequence& a_motionSequence, 
                                                                                                                    const float                                              a_timeSecond, 
                                                                                                                    const std::uint32_t                                      a_motionIndex, 
                                                                                                                    const std::uint32_t                                      a_boneIndex)
{
    return LocalTransform();
}

FWK::Graphics::SkeletalAnimationPlayer::LocalTransform FWK::Graphics::SkeletalAnimationPlayer::InterpolateLocalTransform(const LocalTransform& a_startLocalTransform, const LocalTransform& a_endLcoalTransform, const float a_interpolationWeight) const
{
    return LocalTransform();
}

float FWK::Graphics::SkeletalAnimationPlayer::CalculateAdvancedTimeSecond(const Animation& a_animation, const float a_timeSecond, const float a_deltaTime) const
{
    const auto l_motionDurationSecond = FetchMotionDurationSecond(a_animation);

    // 再生時間が0秒のMotionは時刻を進められないため、
    // 0秒の固定Poseとして扱う
    if (l_motionDurationSecond <= SkeletalAnimationModelRecord::k_initialAnimationDurationSecond) { return SkeletalAnimationModelRecord::k_initialAnimationTimeSecond; }

    auto l_advancedTimeSecond = a_timeSecond + a_deltaTime * a_animation.m_playbackSpeed;

    if (a_animation.m_isLoop)
    {
        // Motionの再生時間を超えた部分を余りとして求め、
		// Motionの有効な時間範囲へ戻す
        l_advancedTimeSecond = std::fmod(l_advancedTimeSecond, l_motionDurationSecond);

        // 負の再生速度ではfmodの結果が負数になることがあるため、
        // Motionの再生時間をs加算して有効な時刻へ戻す
        if (l_advancedTimeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond)
        {
            l_advancedTimeSecond += l_motionDurationSecond;
        }

        return l_advancedTimeSecond;
    }

    // 非LoopAnimationはMotion先頭より前へ進めない
    if (l_advancedTimeSecond <= SkeletalAnimationModelRecord::k_initialAnimationTimeSecond) { return SkeletalAnimationModelRecord::k_initialAnimationTimeSecond; }

    // 非LoopAnimationはMotion終端より先へ進めない
    if (l_advancedTimeSecond >= l_motionDurationSecond) { return l_motionDurationSecond; }

    return l_advancedTimeSecond;
}

void FWK::Graphics::SkeletalAnimationPlayer::CompleteAnimationBlend()
{
    // Blend先Animationを新しい現在Animatoinへ移す
    m_animation = m_blendTargetAnimation;

    // Blend先として更新した再生時刻を、
    // 新しい現在Animationの再生時刻として引き継ぐ
    m_animationTimeSecond = m_blendTargetAnimationTimeSecond;

    // 現在Animationへ移動した後は、
    // Animation自身のBlend時間を使用しない
    m_animation.m_blendDurationSecond = Animation::k_initialBlendDurationSecond;

    m_blendTargetAnimation           = {};
    m_blendTargetAnimationTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
    m_blendElapsedSecond             = k_initialBlendElapsedSecond;

    m_isBlending = false;
}

void FWK::Graphics::SkeletalAnimationPlayer::ResetPlaybackState()
{
    m_animation            = {};
    m_blendTargetAnimation = {};

    m_animationTimeSecond            = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
    m_blendTargetAnimationTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

    m_blendElapsedSecond = k_initialBlendElapsedSecond;

    m_isBlending = false;
}

float FWK::Graphics::SkeletalAnimationPlayer::FetchMotionDurationSecond(const Animation& a_animation) const
{
    const auto& l_skeletalAnimationModelRecord = m_skeletalAnimationModelRecord.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、Motionの再生時間を取得できません。", SkeletalAnimationModelRecord::k_initialAnimationDurationSecond);

    const auto& l_motionSequenceList = l_skeletalAnimationModelRecord->GetREFModelData().m_motionSequenceList;

    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex == Animation::k_invalidMotionIndex, "MotionIndexが無効のため、Motionの再生時間を取得できません。",   SkeletalAnimationModelRecord::k_initialAnimationDurationSecond);
	FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex >= l_motionSequenceList.size(),     "MotionIndexが範囲外のため、Motionの再生時間を取得できません。", SkeletalAnimationModelRecord::k_initialAnimationDurationSecond);

    return l_motionSequenceList[a_animation.m_motionIndex].m_durationSecond;
}