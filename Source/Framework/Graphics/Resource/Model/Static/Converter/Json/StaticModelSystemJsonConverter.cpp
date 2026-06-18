#include "StaticModelSystemJsonConverter.h"

void FWK::Converter::StaticModelSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StaticModelSystem& a_staticModelSystem) const
{
	if (a_rootJson.is_null()) { return; }

	if (a_rootJson.contains(k_staticModelStorageJsonKey))
	{
		auto& l_staticModelSystem = a_staticModelSystem.GetMutableREFStaticModelStorage();

		l_staticModelSystem.Deserialize(a_rootJson[k_staticModelStorageJsonKey]);
	}
}

nlohmann::json FWK::Converter::StaticModelSystemJsonConverter::Serialize(const Graphics::StaticModelSystem& a_staticModelSystem) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_staticModelSystem = a_staticModelSystem.GetREFStaticModelStorage();

	l_rootJson[k_staticModelStorageJsonKey] = l_staticModelSystem.Serialize();

	return l_rootJson;
}