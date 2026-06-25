#include "StandardPipelineStateJsonConverter.h"

void FWK::Converter::StandardPipelineStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardPipelineState& a_standardPipelineState) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::StandardPipelineStateJsonConverter::Serialize(const Graphics::StandardPipelineState& a_standardPipelineState) const
{
	return nlohmann::json();
}