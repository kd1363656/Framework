#include "DepthStencilPassTexture.h"

void FWK::Graphics::DepthStencilPassTexture::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Graphics::DepthStencilPassTexture::Serialize() const
{
	return nlohmann::json();
}

bool FWK::Graphics::DepthStencilPassTexture::Create(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, const Struct::ClientSize& a_clientSize, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
	return false;
}

bool FWK::Graphics::DepthStencilPassTexture::Resize(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, const Struct::ClientSize& a_clientSize, const UINT64& a_retiredFenceValue, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool, ResourceReleaseContext& a_resourceReleaseContext)
{
	return false;
}

UINT FWK::Graphics::DepthStencilPassTexture::FetchVALTextureWidth(const UINT a_clientWidth) const
{
	return 0;
}

UINT FWK::Graphics::DepthStencilPassTexture::FetchVALTextureHeight(const UINT a_clientHeight) const
{
	return 0;
}