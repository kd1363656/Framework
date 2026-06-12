#include "RenderGraphJsonConverter.h"

void FWK::Converter::RenderGraphJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const
{
	if (a_rootJson.is_null()) { return; }

	// レンダーパスのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_renderGraphPassListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializePassList(l_json, a_renderGraph);
	}

	// 描画共通パスのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_drawRequestPassListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializeDrawRequestPassList(l_json, a_renderGraph);
	}

	// 1オブジェクト描画申請クラスのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_drawRequestPerObjectListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializeDrawRequestPerObjectList(l_json, a_renderGraph);
	}
}

nlohmann::json FWK::Converter::RenderGraphJsonConverter::Serialize(const Graphics::RenderGraph & a_renderGraph) const
{
	nlohmann::json l_rootJson = {};

	// レンダーパスのシリアライズ
	l_rootJson[k_renderGraphPassListJsonKey] = SerializePassList(a_renderGraph);

	// 描画共通パスのシリアライズ
	l_rootJson[k_drawRequestPassListJsonKey] = SerializeDrawRequestPassList(a_renderGraph);

	// 1オブジェクト描画申請クラスのシリアライズ
	l_rootJson[k_drawRequestPerObjectListJsonKey] = SerializeDrawRequestPerObjectList(a_renderGraph);

	return l_rootJson;
}

void FWK::Converter::RenderGraphJsonConverter::DeserializePassList(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		std::unique_ptr<Graphics::RenderGraphPassBase> l_renderGraphPass = nullptr;

		Utility::DeserializeInstanceType<TypeAlias::RenderGraphPassUniqueFactory>(l_json, k_renderGraphPassTypeNameJsonKey, l_renderGraphPass);

		if (!l_renderGraphPass)
		{
			assert(false && "RenderGraphPassのインスタンス化に失敗しました。");
			continue;
		}

		a_renderGraph.AddPass(std::move(l_renderGraphPass));
	}
}
void FWK::Converter::RenderGraphJsonConverter::DeserializeDrawRequestPassList(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Graphics::DrawRequestPassBase> l_drawRequestPass = nullptr;

		// ファクトリーからDrawRequestPassを作成する。
		Utility::DeserializeInstanceType<TypeAlias::DrawRequestPassSharedFactory>(l_json, k_drawRequestPassTypeNameJsonKey, l_drawRequestPass);

		if (!l_drawRequestPass)
		{
			assert(false && "DrawRequestPassのインスタンス化に失敗しました。");
			continue;
		}

		a_renderGraph.AddDrawRequestPass(l_drawRequestPass);
	}
}
void FWK::Converter::RenderGraphJsonConverter::DeserializeDrawRequestPerObjectList(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }
	
	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Graphics::DrawRequestPerObjectBase> l_drawRequestPerObject = nullptr;

		// ファクトリーからDrawRequestPassを作成する。
		Utility::DeserializeInstanceType<TypeAlias::DrawRequestPerObjectSharedFactory>(l_json, k_drawRequestPerObjectTypeNameJsonKey, l_drawRequestPerObject);

		if (!l_drawRequestPerObject)
		{
			assert(false && "DrawRequestPerObjectのインスタンス化に失敗しました。");
			continue;
		}

		a_renderGraph.AddDrawRequestPerObject(l_drawRequestPerObject);
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
nlohmann::json FWK::Converter::RenderGraphJsonConverter::SerializeDrawRequestPassList(const Graphics::RenderGraph& a_renderGraph) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_drawRequestPassList = a_renderGraph.GetREFDrawRequestPassList();

	for (const auto& l_drawRequestPass : l_drawRequestPassList)
	{
		if (!l_drawRequestPass) { continue; }

		l_rootJsonArray.emplace_back(Utility::SerializeInstanceType(l_drawRequestPass, k_drawRequestPassTypeNameJsonKey));
	}

	return l_rootJsonArray;
}

nlohmann::json FWK::Converter::RenderGraphJsonConverter::SerializeDrawRequestPerObjectList(const Graphics::RenderGraph& a_renderGraph) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_drawRequestPerObjectList = a_renderGraph.GetREFDrawRequestPerObjectList();

	for (const auto& l_drawRequestPerObject : l_drawRequestPerObjectList)
	{
		if (!l_drawRequestPerObject) { continue; }

		l_rootJsonArray.emplace_back(Utility::SerializeInstanceType(l_drawRequestPerObject, k_drawRequestPerObjectTypeNameJsonKey));
	}

	return l_rootJsonArray;
}