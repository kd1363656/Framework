#include "GraphicsPipelineStateBaseJsonConverter.h"


void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::GraphicsPipelineStateBase& a_graphicsPipelineStateBase) const
{

}

nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::Serialize(const Graphics::GraphicsPipelineStateBase & a_graphicsPipelineStateBase) const
{
	return nlohmann::json();
}

void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::DeserializeRasterizerDesc(const nlohmann::json& a_rootJson, Graphics::GraphicsPipelineStateBase& a_graphicsPipelineStateBase) const
{
}
void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::DeserializeBlendDesc(const nlohmann::json & a_rootJson, Graphics::GraphicsPipelineStateBase & a_graphicsPipelineStateBase) const
{
}
void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::DeserializeDepthStencilDesc(const nlohmann::json & a_rootJson, Graphics::GraphicsPipelineStateBase & a_graphicsPipelineStateBase) const
{
}
void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::DeserializeRTVFormatList(const nlohmann::json & a_rootJson, Graphics::GraphicsPipelineStateBase & a_graphicsPipelineStateBase) const
{
}
void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::DeserializeSampleDesc(const nlohmann::json & a_rootJson, Graphics::GraphicsPipelineStateBase & a_graphicsPipelineStateBase) const
{
}
void FWK::Converter::GraphicsPipelineStateBaseJsonConverter::DeserializeDepthStencilOpDesc(const nlohmann::json & a_rootJson, D3D12_DEPTH_STENCILOP_DESC & a_depthStencilOPDesc) const
{
}

nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::SerializeRasterizerDesc(const Graphics::GraphicsPipelineStateBase & a_graphicsPipelineStateBase) const
{
	return nlohmann::json();
}
nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::SerializeBlendDesc(const Graphics::GraphicsPipelineStateBase& a_graphicsPipelineStateBase) const
{
	return nlohmann::json();
}
nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::SerializeDepthStencilDesc(const Graphics::GraphicsPipelineStateBase& a_graphicsPipelineStateBase) const
{
	return nlohmann::json();
}
nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::SerializeDepthStencilOPDesc(const D3D12_DEPTH_STENCILOP_DESC& a_depthStencilOPDesc) const
{
	return nlohmann::json();
}
nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::SerializeRTVFormatList(const Graphics::GraphicsPipelineStateBase& a_graphicsPipelineStateBase) const
{
	return nlohmann::json();
}
nlohmann::json FWK::Converter::GraphicsPipelineStateBaseJsonConverter::SerializeSampleDesc(const Graphics::GraphicsPipelineStateBase& a_graphicsPipelineStateBase) const
{
	return nlohmann::json();
}