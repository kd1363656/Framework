#include "PhysicsStaticCapsuleBodyJsonConverter.h"

void FWK::Converter::PhysicsStaticCapsuleBodyJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsStaticCapsuleBody& a_physicsStaticCapsuleBody) const
{
	if (a_rootJson.is_null()) { return; }

	a_physicsStaticCapsuleBody.SetHalfHeightOfCylinder(a_rootJson.value(k_halfHeightOfCylinderJsonKey, Constant::k_defaultPhysicsStaticCapsuleBodyHalfHeightOfCylinder));
	a_physicsStaticCapsuleBody.SetRadius              (a_rootJson.value(k_radiusJsonKey, Constant::k_defaultPhysicsStaticCapsuleBodyRadius));
}

nlohmann::json FWK::Converter::PhysicsStaticCapsuleBodyJsonConverter::Serialize(const Physics::PhysicsStaticCapsuleBody& a_physicsStaticCapsuleBody) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_halfHeightOfCylinderJsonKey] = a_physicsStaticCapsuleBody.GetVALHalfHeightOfCylinder();
	l_rootJson[k_radiusJsonKey]               = a_physicsStaticCapsuleBody.GetVALRadius              ();

	return l_rootJson;
}