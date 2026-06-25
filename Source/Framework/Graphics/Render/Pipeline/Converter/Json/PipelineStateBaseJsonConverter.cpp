#include "PipelineStateBaseJsonConverter.h"

void FWK::Converter::PipelineStateBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }
	
}
nlohmann::json FWK::Converter::PipelineStateBaseJsonConverter::Serialize(const Graphics::PipelineState & a_pipelineState) const
{
	nlohmann::json l_rootJson = {};
	
	return l_rootJson;
}