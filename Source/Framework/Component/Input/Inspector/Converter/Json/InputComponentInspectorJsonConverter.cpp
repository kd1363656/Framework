#include "InputComponentInspectorJsonConverter.h"

void FWK::Converter::InputComponentInspectorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, InputComponentInspector& a_inputComponentInspector) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::InputComponentInspectorJsonConverter::Serialize(const InputComponentInspector& a_inputComponentInspector) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}