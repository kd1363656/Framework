#include "PhysicsStaticBoxBodyJsonConverter.h"

void FWK::Converter::PhysicsStaticBoxBodyJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsStaticBoxBody& a_physicsStaticBoxBody) const
{
	if (a_rootJson.is_null()) { return; }

	a_physicsStaticBoxBody.SetHalfExtent(Utility::DeserializeVector3(a_rootJson, k_halfExtentJsonKey));
}

nlohmann::json FWK::Converter::PhysicsStaticBoxBodyJsonConverter::Serialize(const Physics::PhysicsStaticBoxBody & a_physicsStaticBoxBody) const
{
	nlohmann::json l_rootJson = {};

	Utility::SerializeVector3(a_physicsStaticBoxBody.GetRERHalfExtent(), k_halfExtentJsonKey);

	return l_rootJson;
}