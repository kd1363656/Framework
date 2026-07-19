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

    // CPUPose計算に必要なBindPoseと、BoneMotionTrack検索用Dataを作成する
    FWK_ASSERT_RETURN_VALUE_IF(!m_poseEvaluator.Create(l_modelData), "SkeletalAnimationPoseEvaluatorの作成に失敗しました。", false);

    const auto& l_bindPoseGlobalBoneMatrixList = m_poseEvaluator.GetREFBindPoseGlobalBoneMatrixList();

    FWK_ASSERT_RETURN_VALUE_IF(l_bindPoseGlobalBoneMatrixList.size() != l_modelBoneList.size(), "BindPoseGlobalBoneMatrixListの要素数がModelBoneListと一致しません。", false);

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
    m_frameDataList                = std::move                                                  (l_frameDataList);
    
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

    // Motionが設定されていない場合は、
    // Create()またはStop()で設定されたBindPoseを使用する。
	if (m_animation.m_motionIndex == SkeletalAnimationPoseEvaluator::k_invalidMotionIndex) { return; }

    // 現在Animationの再生時刻を更新する
    m_animationTimeSecond = CalculateAdvancedTimeSecond(m_animation, m_animationTimeSecond, a_deltaTime);

    if (m_isBlending)
    {
        // Blend先Animationは現在Animationとは異なる再生速度や
		// Loop設定を持てるため、別の再生時刻として更新する。    
        m_blendTargetAnimationTimeSecond = CalculateAdvancedTimeSecond(m_blendTargetAnimation, m_blendTargetAnimationTimeSecond, a_deltaTime);

        // Blendの進行時間はMotionの再生速度に影響させず、
		// 実際に経過した時間によって進める
        m_blendElapsedSecond += a_deltaTime;

        if (m_blendElapsedSecond >= m_blendTargetAnimation.m_blendDurationSecond)
        {
            CompleteAnimationBlend();
        }
    }
    
    // 更新したAnimation時刻から、
	// 現在FrameResource用のGlobalBoneMatrixをCPUで計算する。
    FWK_ASSERT_RETURN_IF(!EvaluateCurrentPose(), "現在Poseの計算に失敗しました。");
}

bool FWK::Graphics::SkeletalAnimationPlayer::IsAnimationEnd() const
{
    // Motionが設定されいない場合は
    // 再生対象が存在しないため終了状態として扱う
    if (m_animation.m_motionIndex == SkeletalAnimationPoseEvaluator::k_invalidMotionIndex) { return true; }

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

    const auto& l_bindPoseGlobalBoneMatrixList = m_poseEvaluator.GetREFBindPoseGlobalBoneMatrixList();

    // FrameResourceはフレームごとに切り替わる。
	// 現在FrameDataだけをBindPoseへ戻すと、
	// 別のFrameResourceへ切り替わった際に停止前のPoseが再び現れてしまう。
	// そのため、全FrameDataのCPU側GlobalBoneMatrixをBindPoseへ戻す。
    for (auto& l_frameData : m_frameDataList)
    {
        l_frameData.m_globalBoneMatrixList = l_bindPoseGlobalBoneMatrixList;
    }
}

bool FWK::Graphics::SkeletalAnimationPlayer::ApplyAnimation(const Animation& a_animation)
{
    const auto& l_skeletalAnimationModelRecord = m_skeletalAnimationModelRecord.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、Animationを適用できません。", false);

    const auto& l_motionSequenceList = l_skeletalAnimationModelRecord->GetREFModelData().m_motionSequenceList;

    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex == SkeletalAnimationPoseEvaluator::k_invalidMotionIndex, "MotionIndexが無効なため、Animationを適用できません。",   false);
    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex >= l_motionSequenceList.size(),                          "MotionIndexが範囲外のため、Animationを適用できません。", false);

    const auto l_motionDurationSecond = l_motionSequenceList[a_animation.m_motionIndex].m_durationSecond;

    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_startTimeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond,     "AnimationのStartTimeSecondが0未満のため、Animationを適用できません。",                      false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_startTimeSecond > l_motionDurationSecond,                                         "AnimationのStartTimeSecondがMotionの再生時間を超えているため、Animationを適用できません。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_blendDurationSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond, "AnimationのBlendDurationSecondが0未満のため、Animationを適用できません。",                  false);

    if (m_animation.m_motionIndex == SkeletalAnimationPoseEvaluator::k_invalidMotionIndex ||
        m_animation.m_motionIndex >= l_motionSequenceList.size()                          ||
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

bool FWK::Graphics::SkeletalAnimationPlayer::EvaluateCurrentPose()
{
    const auto& l_skeletalAnimationModelRecord = m_skeletalAnimationModelRecord.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、現在Poseを計算できません。", false);

    auto* l_frameData = FetchMutablePTRCurrentFrameData();

    FWK_ASSERT_RETURN_VALUE_IF(!l_frameData, "現在FrameDataを取得できないため、現在Poseを計算できません。", false);

    return m_poseEvaluator.EvaluatePose(l_skeletalAnimationModelRecord->GetREFModelData(),
                                        m_animationTimeSecond,
                                        m_blendTargetAnimationTimeSecond,
                                        FetchVALBlendWeight(),
                                        m_animation.m_motionIndex,
                                        m_blendTargetAnimation.m_motionIndex,
                                        m_isBlending,
                                        l_frameData->m_globalBoneMatrixList);
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

    FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex == SkeletalAnimationPoseEvaluator::k_invalidMotionIndex, "MotionIndexが無効のため、Motionの再生時間を取得できません。",   SkeletalAnimationModelRecord::k_initialAnimationDurationSecond);
	FWK_ASSERT_RETURN_VALUE_IF(a_animation.m_motionIndex >= l_motionSequenceList.size(),                          "MotionIndexが範囲外のため、Motionの再生時間を取得できません。", SkeletalAnimationModelRecord::k_initialAnimationDurationSecond);

    return l_motionSequenceList[a_animation.m_motionIndex].m_durationSecond;
}