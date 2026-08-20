#include "StaticModelComponentJsonConverter.h"

void FWK::Converter::StaticModelComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, StaticModelComponent& a_staticModelComponent) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_registerDrawRequestStrategyMapJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeRegisterDrawRequestStrategyMap(l_json, a_staticModelComponent);
	}
}

nlohmann::json FWK::Converter::StaticModelComponentJsonConverter::SerializePrefab(const StaticModelComponent& a_staticModelComponent) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_registerDrawRequestStrategyMapJsonKey] = SerializeRegisterDrawRequestStrategyMap(a_staticModelComponent);

	return l_rootJson;
}

void FWK::Converter::StaticModelComponentJsonConverter::DeserializeRegisterDrawRequestStrategyMap(const nlohmann::json& a_rootJson, StaticModelComponent& a_staticModelComponent) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson.is_null()))
	{
		return; 
	}

	// 描画要求登録Strategyをデシリアライズ
	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		std::unique_ptr<StaticModelRegisterDrawRequestStrategyBase> l_drawRequestStrategy = nullptr;

		Utility::DeserializeInstanceType<TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory>(l_json, k_drawRequestStrategyJsonKey, l_drawRequestStrategy);

		if (!l_drawRequestStrategy) { continue; }

		a_staticModelComponent.AddRegisterDrawRequestStrategy(std::move(l_drawRequestStrategy));
	}
}

nlohmann::json FWK::Converter::StaticModelComponentJsonConverter::SerializeRegisterDrawRequestStrategyMap(const StaticModelComponent& a_staticModelComponent) const
{
	      auto  l_rootJsonArray                  = nlohmann::json::array                                      ();
	const auto& l_registerDrawRequestStrategyMap = a_staticModelComponent.GetREFRegisterDrawRequestStrategyMap();

	for (const auto& [l_staticTypeID, l_registerDrawRequestStrategy] : l_registerDrawRequestStrategyMap)
	{
		if (!l_registerDrawRequestStrategy) { continue; }

		const auto& l_json = Utility::SerializeInstanceType(l_registerDrawRequestStrategy, k_drawRequestStrategyJsonKey);

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}