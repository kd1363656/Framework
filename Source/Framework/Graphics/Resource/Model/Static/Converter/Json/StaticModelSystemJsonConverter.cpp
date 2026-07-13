#include "StaticModelSystemJsonConverter.h"

void FWK::Converter::StaticModelSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StaticModelSystem& a_staticModelSystem) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_modelStorageJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_modelStorage = a_staticModelSystem.GetMutableREFModelStorage();

		l_modelStorage.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::StaticModelSystemJsonConverter::Serialize(const Graphics::StaticModelSystem& a_staticModelSystem) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_modelStorage = a_staticModelSystem.GetREFModelStorage();

	l_rootJson[k_modelStorageJsonKey] = l_modelStorage.Serialize();

	return l_rootJson;
}