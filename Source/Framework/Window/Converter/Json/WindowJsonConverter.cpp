#include "WindowJsonConverter.h"

void FWK::Converter::WindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Window& a_window) const
{
	if (a_rootJson.is_null()) { return; }

	TypeAlias::TagType l_styleTag = Utility::DeserializeTag(a_rootJson, k_windowStyleTagJsonKey);

	a_window.SetWindowStyleTag(l_styleTag);
}

nlohmann::json FWK::Converter::WindowJsonConverter::Serialize(const Window& a_window) const
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, Utility::SerializeTag(a_window.GetVALWindowStyleTag(), k_windowStyleTagJsonKey));
	
	return l_rootJson;
}