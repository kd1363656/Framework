#include "RenderGraphJsonConverter.h"

void FWK::Converter::RenderGraphJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_renderGraphPassListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializePassList(l_json, a_renderGraph);
	}
}

nlohmann::json FWK::Converter::RenderGraphJsonConverter::Serialize(const Graphics::RenderGraph & a_renderGraph) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_renderGraphPassListJsonKey] = SerializePassList(a_renderGraph);

	return l_rootJson;
}

void FWK::Converter::RenderGraphJsonConverter::DeserializePassList(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const
{
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Graphics::RenderGraphPassBase> l_renderGraphPass = nullptr;

		Utility::DeserializeInstanceType<TypeAlias::RenderGraphPassSharedFactory>(l_json, k_renderGraphPassTypeNameJsonKey, l_renderGraphPass);

		a_renderGraph.AddPass(l_renderGraphPass);
	}
}

nlohmann::json FWK::Converter::RenderGraphJsonConverter::SerializePassList(const Graphics::RenderGraph & a_renderGraph) const
{
	nlohmann::json l_rootJsonArray = nlohmann::json::array();

	const auto& l_passList = a_renderGraph.GetREFPassList();

	for (const auto& l_pass : l_passList)
	{
		if (!l_pass) { continue; }

		nlohmann::json l_json = {};

		Utility::UpdateJson(l_json, Utility::SerializeInstanceType(l_pass, k_renderGraphPassTypeNameJsonKey));

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}