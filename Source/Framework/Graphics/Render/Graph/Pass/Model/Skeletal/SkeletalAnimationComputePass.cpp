#include "SkeletalAnimationComputePass.h"

FWK::Graphics::SkeletalAnimationComputePass::SkeletalAnimationComputePass()
{
	// SkeletalModelの描画より前に、
	// AnimationのBoneMatrix計算を完了させる
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Animation);
}

FWK::Graphics::SkeletalAnimationComputePass::~SkeletalAnimationComputePass() = default;

void FWK::Graphics::SkeletalAnimationComputePass::Execute(Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	// LocalMatrix計算用のComputePipelineStateと
	// RootsignatureをComputeCommandLIstへ設定する
	const auto& l_skeletalAnimationPerObjectComputeRequest = a_renderGraph.FindVALComputeRequestPerObject<SkeletalAnimationPerObjectComputeRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationPerObjectComputeRequest, "SkeletalAnimationPerObjectComputeRequestを取得できないため、SkeletalAnimationComputePassの実行に失敗しました。");

	const auto& l_skeletalAnimationPlayerList = l_skeletalAnimationPerObjectComputeRequest->GetREFSkeletalAnimationPlayerList().GetREFArrayElementDataList();
	const auto& l_computeCommandList          = a_renderer.GetREFComputeCommandList                                          ();

	for (const auto& l_skeletalAnimationPlayerData : l_skeletalAnimationPlayerList)
	{
		const auto& l_skeletalAnimationPlayer = l_skeletalAnimationPlayerData.m_type.lock();

		// BeginFrame後にPlayerが破棄されていた場合は、
		// このPlayerに対するGPU転送を行わない。
		if (!l_skeletalAnimationPlayer) { continue; }

		auto* l_frameData = l_skeletalAnimationPlayer->FetchMutablePTRCurrentFrameData();

		FWK_ASSERT_RETURN_IF(!l_frameData, "現在FrameDataを取得できないため、BoneMatrixをGPUへ転送できません。");

		const auto& l_globalBoneMatrixList     = l_frameData->m_globalBoneMatrixList;
		      auto& l_boneMatrixBuffer         = l_frameData->m_boneMatrixBuffer;
			  auto& l_boneMatrixBufferUploader = l_frameData->m_boneMatrixBufferUploader;

		// CPU側とGPU側でBone数が異なる場合は、
		// 正しいサイズでコピーできない
		FWK_ASSERT_RETURN_IF(l_globalBoneMatrixList.size() != static_cast<std::size_t>(l_boneMatrixBuffer.GetVALElementCount()), "GlobalBoneMatrixListとBoneMatrixBufferの要素数が一致しません。");

		// CPUで計算済みのGlobalBoneMatrixを、
		// 現在FrameDataのUploadBufferへ書き込む。
		FWK_ASSERT_RETURN_IF(!l_boneMatrixBufferUploader.Write( l_globalBoneMatrixList), "BoneMatrixのUploadBufferへの書き込みに失敗しました。");

		const auto& l_boneMatrixBufferResource       = l_boneMatrixBuffer.GetREFBufferGPUResource   ().m_resource;
		const auto& l_boneMatrixUploadBufferResource = l_boneMatrixBufferUploader.GetREFUploadBuffer().GetREFUploadBuffer();

		FWK_ASSERT_RETURN_IF(!l_boneMatrixBufferResource,       "BoneMatrixBufferのGPUResourceが無効です。");
		FWK_ASSERT_RETURN_IF(!l_boneMatrixUploadBufferResource, "BoneMatrix用UploadBufferのGPUResourceが無効です。");

		const auto& l_boneMatrixBufferSize = sizeof(TypeAlias::Math::Matrix) * l_globalBoneMatrixList.size();

		const auto l_currentResourceState = l_boneMatrixBuffer.GetVALCurrentResourceState();

		// UploadBufferからコピーできるようにDefaultヒープ側をCOPY_DESTへ遷移させる
		l_computeCommandList.TransitionResourceBarrier(l_boneMatrixBufferResource, l_currentResourceState, D3D12_RESOURCE_STATE_COPY_DEST);

		l_boneMatrixBuffer.SetCurrentResourceState(D3D12_RESOURCE_STATE_COPY_DEST);

		// Copy命令と後続のCompute命令を
		// 同じComputeCommandListへ記録する。
		// CommandQueueは記録順に実行するため、
		// ここでCPU待機は発生しない。
		l_computeCommandList.CopyBufferRegion(k_boneMatrixBufferCopyDestinationOffset,
			                                  k_boneMatrixBufferCopySourceOffset,
			                                  l_boneMatrixBufferSize,
			                                  *l_boneMatrixBufferResource.Get(),
			                                  *l_boneMatrixUploadBufferResource.Get());

		// コピー後はSkinningComputeShaderからStructuredBufferとして読み取る。
		l_computeCommandList.TransitionResourceBarrier(l_boneMatrixBufferResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		l_boneMatrixBuffer.SetCurrentResourceState    (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
}