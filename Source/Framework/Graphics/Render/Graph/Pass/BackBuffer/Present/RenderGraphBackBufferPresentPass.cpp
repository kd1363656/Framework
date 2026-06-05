#include "RenderGraphBackBufferPresentPass.h"

FWK::Graphics::RenderGraphBackBufferPresentPass::RenderGraphBackBufferPresentPass() : 
	RenderGraphPassBase(Enum::RenderGraphPassType::BackBufferClear)
{
	AddResourceAccess(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceAccessType::Present);
}
FWK::Graphics::RenderGraphBackBufferPresentPass::~RenderGraphBackBufferPresentPass() = default;

void FWK::Graphics::RenderGraphBackBufferPresentPass::Execute(const ResourceContext&, Renderer&)
{
	// ここでは何もしない
	// このPassの役割は、RenderGraphにBackBUfferをPRESENT状態へ戻させること
}