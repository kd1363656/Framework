#include "RenderGraphJsonConverter.h"

void FWK::Converter::RenderGraphJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_swapChain) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::RenderGraphJsonConverter::Serialize(const Graphics::RenderGraph& a_swapChain) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}