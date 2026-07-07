#include "FinalColorPass.h"

FWK::Graphics::FinalColorPass::FinalColorPass()
{
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::PostEffect);

	// SceneColorはLinear色として読む
	ReadRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor, Enum::RenderGraphResourceUsage::PixelShaderResource, Enum::RenderGraphResourceUsage::RenderTarget);

	// FinalColorには、画面表示用にガンマ補正済みの色を書き込む
	WriteRenderTarget(Enum::RenderGraphRenderTargetType::FinalColor, Enum::RenderGraphResourceUsage::RenderTarget);
}
FWK::Graphics::FinalColorPass::~FinalColorPass() = default;

void FWK::Graphics::FinalColorPass::Execute(Renderer & a_renderer, RenderGraph & a_renderGraph)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	const auto& l_rootSignature     = SetupGraphicsRenderPipeline		(a_renderer, Enum::PipelineStateType::FinalColor).lock();

	FWK_ASSERT_RETURN_IF(!l_rootSignature, "FinalColorPass用RootSignatureが無効のため、FinalColorPassの実行に失敗しました。");

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、FinalColorPassの実行に失敗しました。");

	const auto& l_finalColorDrawRequest = a_renderGraph.FindVALDrawRequestPass<FinalColorRenderTargetPassDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_finalColorDrawRequest,																						    "FinalColorRenderTargetPassDrawRequestが無効のため、FinalColorPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF(!l_finalColorDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "FinalColorPass定数バッファの設定に失敗しました。");
}