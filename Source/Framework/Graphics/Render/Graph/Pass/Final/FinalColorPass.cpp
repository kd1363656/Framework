#include "FinalColorPass.h"

FWK::Graphics::FinalColorPass::FinalColorPass()
{
	// SceneColorはLinear色として読む
	ReadResource(Enum::RenderGraphResourceType::SceneColor, Enum::RenderGraphResourceUsage::PixelShaderResource);

	// FinalColorには、画面表示用にガンマ補正済みの色を書き込む
	WriteResource(Enum::RenderGraphResourceType::FinalColor, Enum::RenderGraphResourceUsage::RenderTarget);
}
FWK::Graphics::FinalColorPass::~FinalColorPass() = default;

void FWK::Graphics::FinalColorPass::Execute(const ResourceContext & a_resourceContext, Renderer & a_renderer, RenderGraph & a_renderGraph)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	const auto& l_rootSignature     = SetupRenderPipeline				(a_renderer, Enum::PipelineStateType::FinalColor).lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_rootSignature, "FinalColorPass用RootSignatureが無効のため、FinalColorPassの実行に失敗しました。");

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、FinalColorPassの実行に失敗しました。");

	const auto& l_finalColorDrawRequest = a_renderGraph.FindVALDrawRequestPass<FinalColorRenderTargetPassDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_finalColorDrawRequest,																						   "FinalColorRednerTargetPassDrawRequestが無効のため、FinalColorPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_finalColorDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "FinalColorPass定数バッファの設定に失敗しました。");

	// SceneColor全体を読み、FinalColorへ全画面描画する
	l_directCommandList.DispatchMesh(Constant::k_defaultDispatchMeshThreadGroupCountX, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);
}