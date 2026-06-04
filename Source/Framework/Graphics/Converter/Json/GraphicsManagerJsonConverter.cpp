#include "GraphicsManagerJsonConverter.h"

void FWK::Converter::GraphicsManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::GraphicsManager& a_graphicsManager) const
{
	if (a_rootJson.is_null()) { return; }

	// レンダラーのデシリアライズ
	if (const auto& l_rendererJson = a_rootJson.value(k_rendererJsonKey, nlohmann::json());
		!l_rendererJson.is_null())
	{
		auto& l_renderer = a_graphicsManager.GetMutableREFRenderer();

		l_renderer.Deserialize(l_rendererJson);
	}

	// リソースコンテキストのデシリアライズ
	if (const auto& l_resourceContextJson = a_rootJson.value(k_resourceContextJsonKey, nlohmann::json());
		!l_resourceContextJson.is_null())
	{
		auto& l_resourceContext = a_graphicsManager.GetMutableREFResourceContext();

		l_resourceContext.Deserialize(l_resourceContextJson);
	}
}

nlohmann::json FWK::Converter::GraphicsManagerJsonConverter::Serialize(const Graphics::GraphicsManager& a_graphicsManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_renderer		  = a_graphicsManager.GetREFRenderer	   ();
	const auto& l_resourceContext = a_graphicsManager.GetREFResourceContext();

	// レンダラーのシリアライズ
	l_rootJson[k_rendererJsonKey] = l_renderer.Serialize();

	// リソースコンテキストのシリアライズ
	l_rootJson[k_resourceContextJsonKey] = l_resourceContext.Serialize();

	return l_rootJson;
}