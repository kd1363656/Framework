#include "PhysicsStaticSphereBodyJsonConverter.h"

void FWK::Converter::PhysicsStaticSphereBodyJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsStaticSphereBody& a_physicsStaticSphereBody) const
{
	if (a_rootJson.is_null()) { return; }

	a_physicsStaticSphereBody.SetRadius(a_rootJson.value(k_radiusJsonKey, Constant::k_defaultPhysicsStaticSphereBodyRadius));
}

nlohmann::json FWK::Converter::PhysicsStaticSphereBodyJsonConverter::Serialize(const Physics::PhysicsStaticSphereBody& a_physicsStaticSphereBody) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_radiusJsonKey] = a_physicsStaticSphereBody.GetVALRadius();

	return l_rootJson;
}