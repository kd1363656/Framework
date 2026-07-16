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
	const auto& l_rootSignature         = SetupComputeRenderPipeline           (a_renderer, Enum::PipelineStateType::SkeletalAnimationLocalMatrix).lock();
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                                 ();

	const auto& l_skeletalAnimationPerObjectComputeRequest = a_renderGraph.FindVALComputeRequestPerObject<SkeletalAnimationPerObjectComputeRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_rootSignature,                            "LocalMatrix計算用RootSignatureを取得できないため、SkeletalAnimationComputePassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF(!l_currentFrameResource,                     "現在のFrameResourceを取得できないため、SkeletalAnimationComputePassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationPerObjectComputeRequest, "SkeletalAnimationPerObjectComputeRequestを取得できないため、SkeletalAnimationComputePassの実行に失敗しました。");

	// 登録されている各SkeletalAnimationPlayerについて、
	// KeyFrameから各BoneのLocalMatrixを計算する
	l_skeletalAnimationPerObjectComputeRequest->DispatchLocalMatrix(a_renderer, *l_rootSignature, *l_currentFrameResource);
}