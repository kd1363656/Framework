#include "FPSControllerJsonConverter.h"

void FWK::Converter::FPSControllerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, FPSController& a_fpsController) const
{
	if (a_rootJson.is_null()) { return; }

	const float l_targetFPS = a_rootJson.value(k_targetFPSJsonKey, a_fpsController.GetVALDefaultFPS());

	a_fpsController.SetTargetFPS(l_targetFPS);
}

nlohmann::json FWK::Converter::FPSControllerJsonConverter::Serialize(const FPSController& a_fpsController) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_targetFPSJsonKey] = a_fpsController.GetVALTargetFPS();

	return l_rootJson;
}