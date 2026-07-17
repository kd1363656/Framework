#include "SkeletalAnimationPerObjectComputeRequest.h"

void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::BeginFrame()
{
	// 参照先が破棄されたPlayerを一覧から削除する
	m_skeletalAnimationPlayerList.BeginFrame();
}

void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::DispatchLocalMatrix(const Renderer & a_renderer, const RootSignature & a_rootSignature, const FrameResource & a_frameResource)
{
	const auto& l_computeCommandList = a_renderer.GetREFComputeCommandList();

	for (const auto& l_skeletalAnimationPlayerElement : m_skeletalAnimationPlayerList.GetREFArrayElementDataList())
	{
		const auto& l_skeletalAnimationPlayer = l_skeletalAnimationPlayerElement.m_type.lock();

		if (!l_skeletalAnimationPlayer) { continue; }

		const auto& l_skeletalAnimationModelRecord = l_skeletalAnimationPlayer->GetREFSkeletalAnimationModelRecord().lock();

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、LocalMatrixの計算に失敗しました。");

		auto* const l_frameData = l_skeletalAnimationPlayer->FetchMutablePTRCurrentFrameData();

		FWK_ASSERT_RETURN_IF(!l_frameData, "SkeletalAnimationPlayerの現在FrameDataを取得できないため、LocalMatrixの計算に失敗しました。");

		auto& l_boneMatrixBuffer = l_frameData->m_boneMatrixBuffer;

		const auto& l_boneMatrixBufferResource = l_boneMatrixBuffer.GetREFBufferGPUResource().m_resource;

		FWK_ASSERT_RETURN_IF(!l_boneMatrixBufferResource, "BoneMatrixBufferのGPUResourceが無効なため、LocalMatrixの計算に失敗しました。");

		// 前回使用時のResourceStateから、
		// ComputeShaderが書き込めるUAVへ遷移する
		l_computeCommandList.TransitionResourceBarrier(l_boneMatrixBufferResource, l_boneMatrixBuffer.GetVALCurrentResourceState(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		l_boneMatrixBuffer.SetCurrentResourceState    (D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		const auto& l_animation            = l_skeletalAnimationPlayer->GetREFAnimation           ();
		const auto& l_blendTargetAnimation = l_skeletalAnimationPlayer->GetREFBlendTargetAnimation();

		Struct::CBSkeletalAnimationLocalMatrix l_cbSkeletalAnimationLocalMatrix = {};

		l_cbSkeletalAnimationLocalMatrix.m_blendWeight                             = l_skeletalAnimationPlayer->FetchVALBlendWeight                 ();
		l_cbSkeletalAnimationLocalMatrix.m_animationTimeSecond                     = l_skeletalAnimationPlayer->GetVALAnimationTimeSecond           ();
		l_cbSkeletalAnimationLocalMatrix.m_blendTargetAnimationTimeSecond          = l_skeletalAnimationPlayer->GetVALBlendTargetAnimationTimeSecond();
		l_cbSkeletalAnimationLocalMatrix.m_boneBufferSRVDescriptorIndex            = l_skeletalAnimationModelRecord->GetREFBoneBuffer               ().GetVALSRVDescriptorIndex();
		l_cbSkeletalAnimationLocalMatrix.m_motionSequenceBufferSRVDescriptorIndex  = l_skeletalAnimationModelRecord->GetREFMotionSequenceBuffer     ().GetVALSRVDescriptorIndex();
		l_cbSkeletalAnimationLocalMatrix.m_boneMotionTrackBufferSRVDescriptorIndex = l_skeletalAnimationModelRecord->GetREFBoneMotionTrackBuffer    ().GetVALSRVDescriptorIndex();
		l_cbSkeletalAnimationLocalMatrix.m_keyFrameBufferSRVDescriptorIndex        = l_skeletalAnimationModelRecord->GetREFKeyFrameBuffer           ().GetVALSRVDescriptorIndex();
		l_cbSkeletalAnimationLocalMatrix.m_boneMatrixBufferUAVDescriptorIndex      = l_boneMatrixBuffer.GetVALUAVDescriptorIndex                    ();
		l_cbSkeletalAnimationLocalMatrix.m_motionIndex                             = l_animation.m_motionIndex;
		l_cbSkeletalAnimationLocalMatrix.m_blendTargetMotionIndex                  = l_blendTargetAnimation.m_motionIndex;
		l_cbSkeletalAnimationLocalMatrix.m_boneCount                               = l_boneMatrixBuffer.GetVALElementCount();
		l_cbSkeletalAnimationLocalMatrix.m_isBlending                              = static_cast<std::uint32_t>           (l_skeletalAnimationPlayer->GetVALIsBlending());

		SetupConstantBuffer<SkeletalAnimationLocalMatrixPerObjectDynamicConstantBufferUploader>(l_cbSkeletalAnimationLocalMatrix,
			                                                                                    a_rootSignature,
			                                                                                    l_computeCommandList,
			                                                                                    a_frameResource,
			                                                                                    Enum::RootParameterType::CBSkeletalAnimationLocalMatrix);

		// HLSL側では64ThreadがBoneをストライド処理するため
		// Plyaer単位では1GroupだけDispatchする
		l_computeCommandList.Dispatch(k_singleDispatchThreadGroupCount, k_singleDispatchThreadGroupCount, k_singleDispatchThreadGroupCount);

		// 構造区のHierarchy計算が同じBoneMatrixBufferを読み書きする前に、LocalMatrixの書き込み完了を保証する
		l_computeCommandList.UAVResourceBarrier(l_boneMatrixBufferResource);
	}
}
void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::DispatchBoneHIerarchy(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	const auto& l_computeCommandList = a_renderer.GetREFComputeCommandList();

	for (const auto& l_skeletalAnimatoinPlayerElement : m_skeletalAnimationPlayerList.GetREFArrayElementDataList())
	{
		const auto& l_skeletalAnimationPlayer = l_skeletalAnimatoinPlayerElement.m_type.lock();

		if (!l_skeletalAnimationPlayer) { continue; }

		const auto& l_skeletalAnimationModelRecord = l_skeletalAnimationPlayer->GetREFSkeletalAnimationModelRecord().lock();

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、BoneHierarchyの計算に失敗しました。");

		auto* const l_frameData = l_skeletalAnimationPlayer->FetchPTRCurrentFrameData();

		FWK_ASSERT_RETURN_IF(!l_frameData, "SkeletalAnimationPlayerの現在FrameDataを取得できないため、BoneHierarchyの計算に失敗しました。");

		const auto& l_boneMatrixBuffer         = l_frameData->m_boneMatrixBuffer;
		const auto& l_boneMatrixBufferResource = l_boneMatrixBuffer.GetREFBufferGPUResource().m_resource;

		FWK_ASSERT_RETURN_IF(!l_boneMatrixBufferResource, "BoneMatrixBufferのGPUResourceが無効なため、BoneHierarchyの計算に失敗しました。");

		const auto l_maxBoneHierarchyDepth = l_skeletalAnimationModelRecord->GetVALMAXBoneHierarchyDepth();

		// RootBoneしか存在しない場合、
		// LocalMatrixがそのままGlobalMatrixになる
		if (l_maxBoneHierarchyDepth < k_firstChildHierarchyDepth) { continue; }

		Struct::CBSkeletalAnimationBoneHierarchy l_cbSkeletalAnimationBoneHierarchy = {};

		l_cbSkeletalAnimationBoneHierarchy.m_boneBufferSRVDescriptorIndex       = l_skeletalAnimationModelRecord->GetREFBoneBuffer().GetVALSRVDescriptorIndex();
		l_cbSkeletalAnimationBoneHierarchy.m_boneMatrixBufferUAVDescriptorIndex = l_boneMatrixBuffer.GetVALUAVDescriptorIndex     ();
		l_cbSkeletalAnimationBoneHierarchy.m_boneCount                          = l_boneMatrixBuffer.GetVALElementCount           ();

		// 親BoneのGlobalMatrixが完成してから子Boneを処理する必要があるため、
		// HierarchyDepthごとにDispatchする
		for (std::uint32_t l_hierarchyDepth = k_firstChildHierarchyDepth; l_hierarchyDepth <= l_maxBoneHierarchyDepth; ++l_hierarchyDepth)
		{
			l_cbSkeletalAnimationBoneHierarchy.m_hierarchyDepth = l_hierarchyDepth;

			SetupConstantBuffer<SkeletalAnimationBoneHierarchyPerObjectDynamicConstantBufferUploader>(l_cbSkeletalAnimationBoneHierarchy,
				                                                                                      a_rootSignature,
																									  l_computeCommandList,
																									  a_frameResource,
																									  Enum::RootParameterType::CBSkeletalAnimationBoneHierarchy);

			l_computeCommandList.Dispatch(k_singleDispatchThreadGroupCount, k_singleDispatchThreadGroupCount, k_singleDispatchThreadGroupCount);

			// 次のDepthが現在Depthお結果を親GlobalMatrixとして読むため、
			// Depth間だけUAVBarrierを設定する
			if (l_hierarchyDepth < l_maxBoneHierarchyDepth)
			{
				l_computeCommandList.UAVResourceBarrier(l_boneMatrixBufferResource);
			}
		}
	}
}

void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::AddComputeRequest(const std::shared_ptr<SkeletalAnimationPlayer>&a_skeletalAnimationPlayer)
{
	FWK_ASSERT_RETURN_IF(!a_skeletalAnimationPlayer, "SkeletalAnimationPlayerが無効なため、Compute申請を追加できませんでした。");

	m_skeletalAnimationPlayerList.Add(a_skeletalAnimationPlayer);
}