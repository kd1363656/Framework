#include "SkeletalAnimationModelRecord.h"

bool FWK::Graphics::SkeletalAnimationModelRecord::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue == Constant::k_unusedFenceValue, "FenceValueが無効のため、SkeletalAnimationModelRecordの遅延解放Queue登録に失敗しました。", false);

    for (auto& l_modelMesh : m_modelData.m_modelMeshList)
    {
        auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

        auto& l_vertexBuffer            = l_modelMeshRuntimeData.m_vertexBuffer;
        auto& l_meshletBuffer           = l_modelMeshRuntimeData.m_meshletBuffer;
        auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
        auto& l_primitiveIndexBuffer    = l_modelMeshRuntimeData.m_primitiveIndexBuffer;
        auto& l_meshletBoundsBuffer     = l_modelMeshRuntimeData.m_meshletBoundsBuffer;
        auto& l_bonePaletteBuffer       = l_modelMeshRuntimeData.m_bonePaletteBuffer;

        FWK_ASSERT_RETURN_VALUE_IF(!l_vertexBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),            "SkeletalAnimationModelRecordのVertexBufferを遅延解放Queueへ登録できませんでした。",            false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),           "SkeletalAnimationModelRecordのMeshletBufferを遅延解放Queueへ登録できませんでした。",           false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_uniqueVertexIndexBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "SkeletalAnimationModelRecordのUniqueVertexIndexBufferを遅延解放Queueへ登録できませんでした。", false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_primitiveIndexBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),    "SkeletalAnimationModelRecordのPrimitiveIndexBufferを遅延解放Queueへ登録できませんでした。",    false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBoundsBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),     "SkeletalAnimationModelRecordのMeshletBoundsBufferを遅延解放Queueへ登録できませんでした。",     false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_bonePaletteBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),       "SkeletalAnimationModelRecordのBonePaletteBufferを遅延解放Queueへ登録できませんでした。",       false);

        auto& l_lodCoveredLayerCountBuffer = l_modelMeshRuntimeData.m_lodCoveredLayerCountBuffer;

        FWK_ASSERT_RETURN_VALUE_IF(!l_lodCoveredLayerCountBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "SkeletalAnimationModelRecordのLODCoveredLayerCountBufferを遅延解放Queueへ登録できませんでした。", false);

        // モーフのBufferは、モーフを持たないMeshでは作成していない(作成していないBufferは何もせず成功する)
        auto& l_morphVertexRangeBuffer    = l_modelMeshRuntimeData.m_morphVertexRangeBuffer;
        auto& l_morphDeltaBuffer          = l_modelMeshRuntimeData.m_morphDeltaBuffer;
        auto& l_morphChannelListBuffer    = l_modelMeshRuntimeData.m_morphChannelListBuffer;
        auto& l_morphLODTargetErrorBuffer = l_modelMeshRuntimeData.m_morphLODTargetErrorBuffer;

        FWK_ASSERT_RETURN_VALUE_IF(!l_morphVertexRangeBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "SkeletalAnimationModelRecordのMorphVertexRangeBufferを遅延解放Queueへ登録できませんでした。", false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_morphDeltaBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),       "SkeletalAnimationModelRecordのMorphDeltaBufferを遅延解放Queueへ登録できませんでした。",       false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_morphChannelListBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "SkeletalAnimationModelRecordのMorphChannelListBufferを遅延解放Queueへ登録できませんでした。", false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_morphLODTargetErrorBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "SkeletalAnimationModelRecordのMorphLODTargetErrorBufferを遅延解放Queueへ登録できませんでした。", false);
    }

    return true;
}

std::uint32_t FWK::Graphics::SkeletalAnimationModelRecord::FindVALMorphChannelIndex(const std::wstring& a_channelName) const
{
    const auto& l_morphChannelNameList = m_modelData.m_morphChannelNameList;

    const auto& l_itr = std::ranges::find(l_morphChannelNameList, a_channelName);

    if (l_itr == l_morphChannelNameList.end()) { return Constant::k_invalidModelMorphTargetIndex; }

    return static_cast<std::uint32_t>(std::distance(l_morphChannelNameList.begin(), l_itr));
}