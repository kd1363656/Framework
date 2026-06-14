#include "FinalPresentPass.h"

FWK::Graphics::FinalPresentPass::FinalPresentPass()
{
	// SceneColorではLinerar色が入っている中間レンダーターゲット
	// FinalPresentではSRVとして読み込む。
	ReadResource(Enum::RenderGraphResourceType::SceneColor, Enum::RenderGraphResourceUsage::PixelShaderResource);

	// BackBufferは最終表示先なのでRTVとして書き込む
	WriteResource(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceUsage::RenderTarget);
}
FWK::Graphics::FinalPresentPass::~FinalPresentPass() = default;

void FWK::Graphics::FinalPresentPass::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	const auto& l_direcCommandList = a_renderer.GetREFDirectCommandList();

	// FinalPresent用のPSO/RootSignatureをセットする。
	const auto& l_rootSignature = SetupRenderPipeline(a_renderer, Enum::PipelineStateType::FinalPresent).lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_rootSignature, "FinalPresentPass用RootSignatureが無効のため、FinalPresentPassの実行に失敗しました。");

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、、FinalPresentPassの実行に失敗しました。");

	// FinalPresentはBackBufferに描くので、ここで現在のBackBufferをOMにセットする。
	auto& l_swapChain = a_renderer.GetMutableREFSwapChain();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList   ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(),										 "BackBufferListが空のため、FinalPresentPassの描画先設定に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、FinalPresentPassの描画先設定に失敗しました。");

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、FinalPresentPassの描画先設定に失敗しました。");

	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_renderArea        = a_renderer.GetREFRenderArea			   ();

	// ビューポート、シザー矩形をセット
	l_direcCommandList.SetupRenderArea  (l_renderArea);
	l_direcCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	const auto& l_finalPresentDrawRequest = a_renderGraph.FindVALDrawRequestPass<FinalPresentRenderTargetPassDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_finalPresentDrawRequest,																							"FinalPresentRenderTargetPassDrawRequestが無効のため、FinalPresentPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_finalPresentDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_direcCommandList, *l_currentFrameResource), "FinalPresentPass定数バッファの設定に失敗しました。");

	// 全画面三角形を1毎描画する
	l_direcCommandList.DispatchMesh(Constant::k_defaultDispatchMeshThreadGroupCountX, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);
}