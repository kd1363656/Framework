#include "SceneManagerJsonConveter.h"

void FWK::Converter::SceneManagerJsonConverter::Deserialize (const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const
{
	if (a_rootJson.is_null ()) { return; }


}

nlohmann::json FWK::Converter::SceneManagerJsonConverter::Serialize (const SceneManager& a_sceneManager) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}