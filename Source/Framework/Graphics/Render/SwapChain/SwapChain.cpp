#include "SwapChain.h"

void FWK::Graphics::SwapChain::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::SwapChain::Create(const Window& a_window, const Device& a_device, const Factory& a_factory, const DirectCommandQueue& a_directCommandQueue, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	return false;
}

void FWK::Graphics::SwapChain::Present() const
{
}

nlohmann::json FWK::Graphics::SwapChain::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::SwapChain::Resize(const Device& a_device, const Struct::ClientSize& a_clientSize, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	return false;
}

void FWK::Graphics::SwapChain::ResizeBackBufferList(const std::size_t a_backBufferNUM)
{
}

UINT FWK::Graphics::SwapChain::FetchVALCurrentBackBufferIndex() const
{
	return 0;
}

bool FWK::Graphics::SwapChain::CreateSwapChain(const Window& a_window, const Factory& a_factory, const DirectCommandQueue& a_directCommandQueue)
{
	return false;
}

bool FWK::Graphics::SwapChain::CreateBackBufferList(const Device& a_device, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	return false;
}

void FWK::Graphics::SwapChain::ReleaseBackBufferList(TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
}

bool FWK::Graphics::SwapChain::IsValidBackBufferSize(const Struct::ClientSize& a_clientSize) const
{
	return false;
}
