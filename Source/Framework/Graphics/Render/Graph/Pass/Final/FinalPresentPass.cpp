#include "FinalPresentPass.h"

FWK::Graphics::FinalPresentPass::FinalPresentPass()
{
	// シーンカラー用レンダーターゲットテクスチャのリソース状態をPIXEL_SHADER_RESOURCEに遷移してから読み取る
	ReadResource(Enum::RenderGraphResourceType::FinalColor, Enum::RenderGraphResourceUsage::PixelShaderResource);

	// BackBufferのリソース状態をRENDER_TARGETにして書き込む
	WriteResource(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceUsage::RenderTarget);
}
FWK::Graphics::FinalPresentPass::~FinalPresentPass() = default;

void FWK::Graphics::FinalPresentPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// FinalPresent用のPSO/RootSignatureをセットする。
	const auto& l_rootSignature = SetupRenderPipeline(a_renderer, Enum::PipelineStateType::FinalPresent).lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_rootSignature, "FinalPresentPass用RootSignatureが無効のため、FinalPresentPassの実行に失敗しました。");

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、FinalPresentPassの実行に失敗しました。");

	const auto& l_finalPresentDrawRequest = a_renderGraph.FindVALDrawRequestPass<FinalPresentRenderTargetPassDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_finalPresentDrawRequest,																							 "FinalPresentRenderTargetPassDrawRequestが無効のため、FinalPresentPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_finalPresentDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "FinalPresentPass定数バッファの設定に失敗しました。");

	// 全画面に三角形を1枚描画する
	l_directCommandList.DispatchMesh(Constant::k_defaultDispatchMeshThreadGroupCountX, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);
}