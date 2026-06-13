#include "RenderGraphFrameResource.h"

void FWK::Graphics::RenderGraphFrameResource::Deserialzie(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::RenderGraphFrameResource::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::RenderGraphFrameResource::Create(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, const Struct::ClientSize& a_clientSize, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	return false;
}

bool FWK::Graphics::RenderGraphFrameResource::Resize(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, const Struct::ClientSize& a_clientSize, const UINT64& a_retiredFenceValue, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool, ResourceReleaseContext& a_resourceReleaseContext)
{
	return false;
}

void FWK::Graphics::RenderGraphFrameResource::AddRenderTArgetPassTexture(const std::shared_ptr<RenderTargetPassTexture>& a_renderTargetTexture)
{}

std::weak_ptr<FWK::Graphics::RenderTargetPassTexture> FWK::Graphics::RenderGraphFrameResource::FindVALRenderTargetPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const
{
	return std::weak_ptr<RenderTargetPassTexture>();
}