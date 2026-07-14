#include "SkeletalAnimationPlayer.h"

bool FWK::Graphics::SkeletalAnimationPlayer::Create(const SkeletalAnimationModel& a_skeletalAnimationModel)
{
    FWK_ASSERT_RETURN_VALUE_IF(!a_skeletalAnimationModel.IsValid(), "SkeletalAnimationModelが無効のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    const auto& l_skeletalAnimationModelRecord = a_skeletalAnimationModel.GetREFSkeletalAnimationModelRecord().lock();
    
    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    const auto& l_modelBoneList = l_skeletalAnimationModelRecord->GetREFModelData().m_boneList;

    FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(), "ModelBoneListが空のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    auto& l_graphicsManager = GraphicsManager::GetInstance();

    const auto& l_device   = l_graphicsManager.GetREFDevice  ();
    const auto& l_renderer = l_graphicsManager.GetREFRenderer();

    auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();

    const auto& l_gpuMemoryAllocator      = l_resourceContext.GetREFGPUMemoryAllocator            ();
          auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_frameResourceList = l_renderer.GetREFFrameResourceList();

    FWK_ASSERT_RETURN_VALUE_IF(l_frameResourceList.empty(), "FrameResourceListが空のため、SkeletalAnimationPlayerの作成に失敗しました。", false);

    const auto& l_boneCount = l_modelBoneList.size();

    // Plyaerの再作成途中で失敗しても、
    // 現在Playerが保持している正常なBufferを壊さないように、
    // 新しいBufferは一度ローカル変数へ作成する
    std::vector<DynamicRWStructuredBuffer> l_boneMatrixBufferList = {};

    // フレームリソースの数だけ容量を予約
    l_boneMatrixBufferList.reserve(l_frameResourceList.size());

    
    return false;
}

bool FWK::Graphics::SkeletalAnimationPlayer::PlayMotion(const std::uint32_t a_motionIndex, const bool a_isLoop, const float a_playbackSpeed)
{
    return false;
}

void FWK::Graphics::SkeletalAnimationPlayer::AdvanceTime(const float a_deltaTime)
{

}

bool FWK::Graphics::SkeletalAnimationPlayer::IsAnimationEnd() const
{
    return false;
}

void FWK::Graphics::SkeletalAnimationPlayer::Stop()
{

}

bool FWK::Graphics::SkeletalAnimationPlayer::IsValid() const
{
    return false;
}

bool FWK::Graphics::SkeletalAnimationPlayer::ApplyAnimation(const Animation& a_animation)
{
    return false;
}

bool FWK::Graphics::SkeletalAnimationPlayer::IsValidAnimation(const Animation& a_animation) const
{
    return false;
}

float FWK::Graphics::SkeletalAnimationPlayer::FetchMotionDurationSecond(const Animation& a_animation) const
{
    return 0.0f;
}

float FWK::Graphics::SkeletalAnimationPlayer::CalculateAdvancedTimeSecond(const Animation& a_animation, const float a_timeSecond, const float a_deltaTime) const
{
    return 0.0f;
}

float FWK::Graphics::SkeletalAnimationPlayer::CompleteAnimationBlend()
{
    return 0.0f;
}

void FWK::Graphics::SkeletalAnimationPlayer::ResetPlaybackState()
{

}