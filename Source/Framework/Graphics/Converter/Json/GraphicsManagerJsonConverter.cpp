#include "GraphicsManagerJsonConverter.h"

void FWK::Converter::GraphicsManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::GraphicsManager& a_graphicsManager) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_rendererRootJson = a_rootJson.value(k_rendererJsonKey, nlohmann::json());

	// レンダラーのデシリアライズ
	if (!l_rendererRootJson.is_null())
	{
		auto& l_renderer = a_graphicsManager.GetMutableREFRenderer();

		l_renderer.Deserialize(a_rootJson[k_rendererJsonKey]);
	}
}

nlohmann::json FWK::Converter::GraphicsManagerJsonConverter::Serialize(const Graphics::GraphicsManager& a_graphicsManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_renderer = a_graphicsManager.GetREFRenderer();
	
	// レンダラーのシリアライズ
	l_rootJson[k_rendererJsonKey] = l_renderer.Serialize();

	return l_rootJson;
}