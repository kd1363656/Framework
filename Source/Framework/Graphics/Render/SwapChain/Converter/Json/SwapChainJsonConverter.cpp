#include "SwapChainJsonConverter.h"

void FWK::Converter::SwapChainJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::SwapChain& a_swapChain) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_backBufferNUM       = a_rootJson.value(k_backBufferNUMJsonKey, static_cast<std::size_t>(Graphics::SwapChain::GetVALDefaultBackBufferNUM()));
	const auto  l_defaultSyncInterval = a_rootJson.value(k_syncIntervalJsonKey,  Graphics::SwapChain::GetVALDefaultSyncInterval());

	a_swapChain.ResizeBackBufferList(l_backBufferNUM);
	a_swapChain.SetSyncInterval     (l_defaultSyncInterval);
}

nlohmann::json FWK::Converter::SwapChainJsonConverter::Serialize(const Graphics::SwapChain& a_swapChain) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_backBufferNUMJsonKey] = a_swapChain.GetREFBackBufferList().size();
	l_rootJson[k_syncIntervalJsonKey]  = a_swapChain.GetVALSyncInterval  ();

	return l_rootJson;
}