#include "SpriteScreenPass.h"

FWK::Graphics::ScreenSpritePass::ScreenSpritePass()
{
	WriteResource(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceUsage::RenderTarget);
}
FWK::Graphics::ScreenSpritePass::~ScreenSpritePass() = default;

void FWK::Graphics::ScreenSpritePass::Execute(const ResourceContext& a_resourceContext, const Renderer& a_renderer)
{
	const auto& l_renderGraph = a_renderer.GetREFRenderGraph();

	// ResourceDescriptorHeap[]をHLSL側で使うため
	// SRV用DescriptorHeapをコマンドリストに設定する
	const auto& l_spriteScreenPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<SpriteScreenPerObjectDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_spriteScreenPerObjectDrawRequest, "SpriteScreenPerObjectDrawRequestが無効のため、ScreenSpritePassの実行に失敗しました。");

	l_spriteScreenPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_resourceContext, a_renderer);
}