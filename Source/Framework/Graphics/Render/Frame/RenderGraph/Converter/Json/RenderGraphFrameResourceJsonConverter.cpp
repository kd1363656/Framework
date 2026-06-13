#include "RenderGraphFrameResourceJsonConverter.h"

void FWK::Converter::RenderGraphFrameResourceJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_renderTargetPassTextureListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializeRenderTargetPassTextureList(l_json, a_renderGraphFrameResource);
	}
}

nlohmann::json FWK::Converter::RenderGraphFrameResourceJsonConverter::Serialize(const Graphics::RenderGraphFrameResource & a_renderGraphFrameResource) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_renderTargetPassTextureListJsonKey] = SerializeRenderTargetPassTextureList(a_renderGraphFrameResource);

	return l_rootJson;
}

void FWK::Converter::RenderGraphFrameResourceJsonConverter::DeserializeRenderTargetPassTextureList(const nlohmann::json& a_rootJson, Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		const auto& l_renderTargetPassTexture = std::make_shared<Graphics::RenderTargetPassTexture>();

		l_renderTargetPassTexture->Deserialize(l_json);

		a_renderGraphFrameResource.AddRenderTargetPassTexture(l_renderTargetPassTexture);
	}
}

nlohmann::json FWK::Converter::RenderGraphFrameResourceJsonConverter::SerializeRenderTargetPassTextureList(const Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_renderTargetPassTextureList = a_renderGraphFrameResource.GetREFRenderTargetPassTextureList();

	for (const auto& l_renderTargetPassTexture : l_renderTargetPassTextureList)
	{
		if (!l_renderTargetPassTexture) { continue; }

		l_rootJsonArray.emplace_back(l_renderTargetPassTexture->Serialize());
	}

	return l_rootJsonArray;
}