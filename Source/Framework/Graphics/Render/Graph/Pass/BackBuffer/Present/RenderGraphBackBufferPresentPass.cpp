#include "RenderGraphBackBufferPresentPass.h"

FWK::Graphics::RenderGraphBackBufferPresentPass::RenderGraphBackBufferPresentPass() : 
	RenderGraphPassBase(Enum::RenderGraphPassType::BackBufferPresent)
{
	AddResourceAccess(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceAccessType::Present);
}
FWK::Graphics::RenderGraphBackBufferPresentPass::~RenderGraphBackBufferPresentPass() = default;