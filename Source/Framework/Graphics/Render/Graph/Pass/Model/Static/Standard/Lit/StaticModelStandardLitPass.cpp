#include "StaticModelStandardLitPass.h"

FWK::Graphics::StaticModelStandardLitPass::StaticModelStandardLitPass()
{
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

	// シーンカラー用レンダーターゲットテクスチャのリソース状態をRENDER_TARGETに遷移してから
	// シーンカラーテクスチャに書き込む
	WriteRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor, Enum::RenderGraphResourceUsage::RenderTarget);

	// シーン深度用デプスステンシルテクスチャのリソース状態をDEPTH_WRITEにしてから
	// シーン深度へDepthを書き込む
	WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::StaticModelStandardLitPass::~StaticModelStandardLitPass() = default;

void FWK::Graphics::StaticModelStandardLitPass::Execute(Renderer & a_renderer, RenderGraph & a_renderGraph)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// パイプラインステート、ルートシグネチャをセット
	const auto& l_rootSignature		   = SetupRenderPipeline				  (a_renderer, Enum::PipelineStateType::StaticModelUnLit).lock();
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock												      ();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のフレームリソースの取得に失敗しており、StaticModelStandardUnLitPassの実行に失敗しました。");

	const auto& l_cameraPassDrawRequest			          = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>					      ().lock();
	const auto& l_lightPassDrawRequest			          = a_renderGraph.FindVALDrawRequestPass<LightPassDrawRequest>					      ().lock();
	const auto& l_staticModelStandardPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<StaticModelStandardPerObjectDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_cameraPassDrawRequest,																						      "カメラパスのポインタが無効になっており、StaticModelStandardUnLitPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_lightPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),     "ライト定数の設定が出来ておらず、StaticModelStandardUnLitPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_cameraPassDrawRequest->SetupPassConstantBuffer   (*l_rootSignature, l_directCommandList, *l_currentFrameResource), "カメラ定数の設定が出来ておらず、StaticModelStandardUnLitPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_staticModelStandardPerObjectDrawRequest,																		      "StaticModelStandardPerObjectDrawRequestが無効のため、StaticModelStandardUnLitPassの実行に失敗しました。");

	l_staticModelStandardPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);		
}