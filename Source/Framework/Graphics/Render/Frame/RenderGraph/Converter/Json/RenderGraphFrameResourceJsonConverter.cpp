#include "RenderGraphFrameResourceJsonConverter.h"

void FWK::Converter::RenderGraphFrameResourceJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::RenderGraphFrameResourceJsonConverter::Serialize(const Graphics::RenderGraphFrameResource & a_renderGraphFrameResource) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}