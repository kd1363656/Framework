#include "SceneJsonConveter.h"

void FWK::Converter::SceneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (!a_rootJson.is_null()) { return; }
}
nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(const Scene& a_scene) const
{
	auto l_rootJson = nlohmann::json{};

	return l_rootJson;
}