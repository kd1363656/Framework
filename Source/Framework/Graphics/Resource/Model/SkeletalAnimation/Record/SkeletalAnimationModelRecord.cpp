#include "SkeletalAnimationModelRecord.h"

void FWK::Graphics::SkeletalAnimationModelRecord::ApplySharedStructuredBuffers(const std::uint32_t           a_maxBoneHierarchyDepth, 
	                                                                                 StaticStructuredBuffer&& a_boneBuffer,
	                                                                                 StaticStructuredBuffer&& a_motionSequenceBuffer,
	                                                                                 StaticStructuredBuffer&& a_boneMotionTrackBuffer,
	                                                                                 StaticStructuredBuffer&& a_keyFrameBuffer)
{
	m_boneBuffer            = std::move(a_boneBuffer);
	m_motionSequenceBuffer  = std::move(a_motionSequenceBuffer);
	m_boneMotionTrackBuffer = std::move(a_boneMotionTrackBuffer);
	m_keyFrameBuffer        = std::move(a_keyFrameBuffer);

	m_maxBoneHierarchyDepth = a_maxBoneHierarchyDepth;
}

bool FWK::Graphics::SkeletalAnimationModelRecord::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue == Fence::k_unusedFenceValue, "FenceValueが無効のため、SkeletalAnimationModelRecordの遅延解放Queue登録に失敗しました。", false);

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
	}

	// 以下の4個はMesh単位ではなく、
    // SkeletalAnimationModel Asset全体で共有しているBufferなので、
    // Meshの解放ループとは分けて1回だけ遅延解放へ登録する
    FWK_ASSERT_RETURN_VALUE_IF(!m_boneBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),            "SkeletalAnimationModelRecordのBoneBufferを遅延解放Queueへ登録できませんでした。",            false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_motionSequenceBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),  "SkeletalAnimationModelRecordのMotionSequenceBufferを遅延解放Queueへ登録できませんでした。",  false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_boneMotionTrackBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "SkeletalAnimationModelRecordのBoneMotionTrackBufferを遅延解放Queueへ登録できませんでした。", false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_keyFrameBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext),        "SkeletalAnimationModelRecordのKeyFrameBufferを遅延解放Queueへ登録できませんでした。",        false);

	return true;
}