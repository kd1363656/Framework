#include "StaticModelComponent.h"

void FWK::StaticModelComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	ModelComponentBase::DeserializePrefab(a_rootJson);
}

nlohmann::json FWK::StaticModelComponent::SerializePrefab()
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, ModelComponentBase::SerializePrefab());

	return l_rootJson;
}