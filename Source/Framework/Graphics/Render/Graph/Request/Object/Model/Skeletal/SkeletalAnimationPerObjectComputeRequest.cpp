#include "SkeletalAnimationPerObjectComputeRequest.h"

void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::BeginFrame()
{
    // 参照先が破棄されたPlayerを一覧から削除する
    m_skeletalAnimationPlayerSmartPointerVectorList.RemoveExpiredElements();
}

void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::AddComputeRequest(const std::shared_ptr<SkeletalAnimationPlayer>&a_skeletalAnimationPlayer)
{
    FWK_ASSERT_RETURN_IF(!a_skeletalAnimationPlayer, "SkeletalAnimationPlayerが無効なため、Compute申請を追加できませんでした。");

    m_skeletalAnimationPlayerSmartPointerVectorList.Add(a_skeletalAnimationPlayer);
}