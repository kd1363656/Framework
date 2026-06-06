#include "RenderGraphBackBufferPresentPass.h"

FWK::Graphics::RenderGraphBackBufferPresentPass::RenderGraphBackBufferPresentPass() : 
	RenderGraphPassBase(Enum::RenderGraphPassType::BackBufferClear)
{
	AddResourceAccess(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceAccessType::Present);
}
FWK::Graphics::RenderGraphBackBufferPresentPass::~RenderGraphBackBufferPresentPass() = default;