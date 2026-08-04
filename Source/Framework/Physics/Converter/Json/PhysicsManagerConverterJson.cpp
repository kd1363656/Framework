#include "PhysicsManagerConverterJson.h"

void FWK::Converter::PhyisicsManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsManager& a_physicsManager) const
{
	if (a_rootJson.is_null()) { return; }

	const bool l_isDisableDebugDrawJsonKey = a_rootJson.value(k_isDisableDebugDrawJsonKey, Constant::k_physicsManagerInitialIsDisableDebugDraw);

	a_physicsManager.SetIsDisableDebugDraw(l_isDisableDebugDrawJsonKey);
}

nlohmann::json FWK::Converter::PhyisicsManagerJsonConverter::Serialize(const Physics::PhysicsManager& a_physicsManager) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_isDisableDebugDrawJsonKey] = a_physicsManager.GetVALIsDisableDebugDraw();

	return l_rootJson;
}