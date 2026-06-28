#include "WindowJsonConverter.h"

void FWK::Converter::WindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Window& a_window) const
{
	if (a_rootJson.is_null()) { return; }

	const Enum::WindowStyle l_windowStyle = a_rootJson.value(k_styleTagJsonKey, Enum::WindowStyle::Normal);

	a_window.SetStyle(l_windowStyle);
}

nlohmann::json FWK::Converter::WindowJsonConverter::Serialize(const Window& a_window) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_styleTagJsonKey] = a_window.GetVALStyle();

	return l_rootJson;
}