#include "SpriteScreenPass.h"

FWK::Graphics::ScreenSpritePass::ScreenSpritePass()
{
	WriteResource(Enum::RenderGraphResourceType::SceneColor, Enum::RenderGraphResourceUsage::RenderTarget);
}
FWK::Graphics::ScreenSpritePass::~ScreenSpritePass() = default;

void FWK::Graphics::ScreenSpritePass::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// パイプラインステート、ルートシグネチャをセット
	const auto& l_rootSignature		   = SetupRenderPipeline				  (a_renderer, Enum::PipelineStateType::SpriteScreen).lock();
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock												  ();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のフレームリソースの取得に失敗しており、ScreenSpritePassの実行に失敗しました。");

	const auto& l_spritePassDrawRequest			   = a_renderGraph.FindVALDrawRequestPass<SpriteScreenPassDrawRequest>          ().lock();
	const auto& l_spriteScreenPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SpriteScreenPerObjectDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_spritePassDrawRequest,																						   "スプライトパスのポインタが無効になっており、ScreenSpritePassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_spritePassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "スプライト定数の設定が出来ておらず、ScreenSpritePassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_spriteScreenPerObjectDrawRequest,																				   "SpriteScreenPerObjectDrawRequestが無効のため、ScreenSpritePassの実行に失敗しました。");

	l_spriteScreenPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_resourceContext, 
																	 a_renderer,
																	 *l_rootSignature,
																     *l_currentFrameResource);
}